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
#ifndef SWIRLY_UTIL_EXCEPTION_HPP
#define SWIRLY_UTIL_EXCEPTION_HPP

#include <swirly/util/Limits.hpp>
#include <swirly/util/Stream.hpp>

#include <cstring> // strcpy()
#include <exception>

namespace swirly {
inline namespace util {

using ErrMsg = StaticStream<MaxErrMsg>;

class SWIRLY_API Exception : public std::exception {
  public:
    explicit Exception(std::string_view what) noexcept;

    ~Exception() override;

    // Copy.
    Exception(const Exception& rhs) noexcept { *this = rhs; }
    Exception& operator=(const Exception& rhs) noexcept
    {
        std::strcpy(what_, rhs.what_);
        return *this;
    }

    // Move.
    Exception(Exception&&) noexcept = default;
    Exception& operator=(Exception&&) noexcept = default;

    const char* what() const noexcept override;

  private:
    char what_[MaxErrMsg + 1];
};

/**
 * Thread-local error message. This thread-local instance of StaticStream can be used to format
 * error messages before throwing. Note that the StaticStream is reset each time this function is
 * called.
 */
SWIRLY_API ErrMsg& errMsg() noexcept;

} // namespace util
} // namespace swirly

#endif // SWIRLY_UTIL_EXCEPTION_HPP
