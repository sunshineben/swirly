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
#ifndef SWIRLYD_HTTPSESS_HPP
#define SWIRLYD_HTTPSESS_HPP

#include <swirly/web/HttpParser.hpp>
#include <swirly/web/HttpRequest.hpp>
#include <swirly/web/HttpResponse.hpp>

#include <swirly/sys/MemAlloc.hpp>
#include <swirly/sys/TcpAcceptor.hpp>

#include <swirly/util/Log.hpp>

#include <boost/intrusive/list.hpp>

namespace swirly {

class RestServ;

class SWIRLY_API HttpSess
: public BasicHttpParser<HttpSess>
, public MemAlloc {
    friend class BasicHttpParser<HttpSess>;
    using AutoUnlinkOption = boost::intrusive::link_mode<boost::intrusive::auto_unlink>;

  public:
    HttpSess(Reactor& r, IoSocket&& sock, const TcpEndpoint& ep, RestServ& rs, Time now);
    ~HttpSess();

    // Copy.
    HttpSess(const HttpSess&) = delete;
    HttpSess& operator=(const HttpSess&) = delete;

    // Move.
    HttpSess(HttpSess&&) = delete;
    HttpSess& operator=(HttpSess&&) = delete;

    void close() noexcept;
    void onInput(int fd, unsigned events, Time now);
    void onTimer(Timer& tmr, Time now);

    boost::intrusive::list_member_hook<AutoUnlinkOption> listHook;

  protected:
    bool doMessageBegin() noexcept { return true; }
    bool doUrl(std::string_view sv) noexcept;
    bool doStatus(std::string_view sv) noexcept
    {
        // Only supported for HTTP responses.
        return false;
    }
    bool doHeaderField(std::string_view sv, bool first) noexcept;
    bool doHeaderValue(std::string_view sv, bool first) noexcept;
    bool doHeadersEnd() noexcept;
    bool doBody(std::string_view sv) noexcept;
    bool doMessageEnd() noexcept;
    bool doChunkHeader(size_t len) noexcept { return true; }
    bool doChunkEnd() noexcept { return true; }

    LogMsg& logMsg() const noexcept
    {
        auto& ref = swirly::logMsg();
        ref << '<' << ep_ << "> "sv;
        return ref;
    }

  private:
    Reactor& reactor_;
    IoSocket sock_;
    TcpEndpoint ep_;
    RestServ& restServ_;
    Reactor::Handle sub_;
    Timer tmr_;
    HttpRequest req_;
    Buffer outbuf_;
    HttpResponse rsp_{outbuf_};
};

} // namespace swirly

#endif // SWIRLYD_HTTPSESS_HPP
