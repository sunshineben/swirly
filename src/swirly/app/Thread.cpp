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
#include "Thread.hpp"

#include <swirly/sys/Reactor.hpp>

namespace swirly {
inline namespace app {
using namespace std;
namespace {
void runReactor(Reactor& r, ThreadConfig config, const std::atomic<bool>& stop)
{
    sigBlockAll();
    pthread_setname_np(pthread_self(), config.name.c_str());
    SWIRLY_NOTICE << "started "sv << config.name << " thread"sv;
    try {
        while (!stop.load(std::memory_order_acquire)) {
            r.poll();
        }
    } catch (const std::exception& e) {
        SWIRLY_ERROR << "exception: "sv << e.what();
        kill(getpid(), SIGTERM);
    }
    SWIRLY_NOTICE << "stopping "sv << config.name << " thread"sv;
}
} // namespace

ReactorThread::ReactorThread(Reactor& r, ThreadConfig config)
: reactor_(r)
, thread_{runReactor, std::ref(r), config, std::cref(stop_)}
{
}

ReactorThread::~ReactorThread()
{
    stop_.store(true, std::memory_order_release);
    reactor_.interrupt();
    thread_.join();
}

} // namespace app
} // namespace swirly
