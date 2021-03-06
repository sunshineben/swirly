/*
 * The Restful Matching-Engine.
 * Copyright (C) 2013, 2018 Swirly Cloud Limited.
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include "EpollReactor.hpp"

#include <swirly/util/Log.hpp>

namespace swirly {
inline namespace sys {
using namespace std;
namespace {

constexpr size_t MaxEvents{16};

} // namespace

EpollReactor::EpollReactor(size_t sizeHint)
: mux_{sizeHint}
{
    const auto fd = efd_.fd();
    data_.resize(max<size_t>(fd + 1, sizeHint));
    mux_.subscribe(fd, 0, EventIn);

    auto& ref = data_[fd];
    ref.sid = 0;
    ref.events = EventIn;
    ref.slot = {};
}

EpollReactor::~EpollReactor()
{
    mux_.unsubscribe(efd_.fd());
}

void EpollReactor::doInterrupt() noexcept
{
    // Best effort.
    std::error_code ec;
    efd_.write(1, ec);
}

Reactor::Handle EpollReactor::doSubscribe(int fd, unsigned events, IoSlot slot)
{
    assert(fd >= 0);
    assert(slot);
    if (fd >= static_cast<int>(data_.size())) {
        data_.resize(fd + 1);
    }
    auto& ref = data_[fd];
    mux_.subscribe(fd, ++ref.sid, events);
    ref.events = events;
    ref.slot = slot;
    return {*this, fd, ref.sid};
}

void EpollReactor::doUnsubscribe(int fd, int sid) noexcept
{
    auto& ref = data_[fd];
    if (ref.sid == sid) {
        mux_.unsubscribe(fd);
        ref.events = 0;
        ref.slot.reset();
    }
}

void EpollReactor::doSetEvents(int fd, int sid, unsigned events, IoSlot slot)
{
    auto& ref = data_[fd];
    if (ref.sid == sid) {
        if (ref.events != events) {
            mux_.setEvents(fd, sid, events);
            ref.events = events;
        }
        ref.slot = slot;
    }
}

void EpollReactor::doSetEvents(int fd, int sid, unsigned events)
{
    auto& ref = data_[fd];
    if (ref.sid == sid && ref.events != events) {
        mux_.setEvents(fd, sid, events);
        ref.events = events;
    }
}

Timer EpollReactor::doTimer(Time expiry, Duration interval, Priority priority, TimerSlot slot)
{
    return tqs_[static_cast<size_t>(priority)].insert(expiry, interval, slot);
}

Timer EpollReactor::doTimer(Time expiry, Priority priority, TimerSlot slot)
{
    return tqs_[static_cast<size_t>(priority)].insert(expiry, slot);
}

int EpollReactor::doPoll(Time now, Millis timeout)
{
    enum { High = 0, Low = 1 };
    using namespace chrono;

    for (const auto& tq : tqs_) {
        if (!tq.empty()) {
            // Millis until next expiry.
            const auto expiry = duration_cast<Millis>(tq.front().expiry() - now);
            if (expiry < timeout) {
                timeout = max(expiry, 0ms);
            }
        }
    }
    Event buf[MaxEvents];
    error_code ec;
    const auto ret = mux_.wait(buf, MaxEvents, timeout, ec);
    if (ret < 0) {
        if (ec.value() != EINTR) {
            throw system_error{ec};
        }
        return 0;
    }
    now = UnixClock::now();
    const auto n = tqs_[High].dispatch(now) + dispatch(buf, ret, now);
    // Low priority timers are only dispatched during empty cycles.
    return n == 0 ? tqs_[Low].dispatch(now) : n;
}

int EpollReactor::dispatch(Event* buf, int size, Time now)
{
    int n{0};
    for (int i{0}; i < size; ++i) {

        auto& ev = buf[i];
        const auto fd = mux_.fd(ev);
        if (fd == efd_.fd()) {
            SWIRLY_INFO << "reactor interrupted"sv;
            efd_.read();
            continue;
        }
        const auto& ref = data_[fd];

        const auto sid = mux_.sid(ev);
        // Skip this socket if it was modified after the call to wait().
        if (ref.sid > sid) {
            continue;
        }
        // Apply the interest events to filter-out any events that the user may have removed from
        // the events since the call to wait() was made. This would typically happen via a reentrant
        // call into the reactor from an event-handler.
        const auto events = mux_.events(ev) & ref.events;
        if (!events) {
            continue;
        }

        try {
            ref.slot(fd, events, now);
        } catch (const std::exception& e) {
            using namespace string_literals;
            SWIRLY_ERROR << "error handling io event: "sv << e.what();
        }
        ++n;
    }
    return n;
}

} // namespace sys
} // namespace swirly
