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
#ifndef SWIRLY_SQLITE_TYPES_HPP
#define SWIRLY_SQLITE_TYPES_HPP

#include <memory>

#include <sqlite3.h>

namespace swirly {
inline namespace sqlite {

using DbPtr = std::unique_ptr<sqlite3, int (*)(sqlite3*)>;
using StmtPtr = std::unique_ptr<sqlite3_stmt, int (*)(sqlite3_stmt*)>;

} // namespace sqlite
} // namespace swirly

#endif // SWIRLY_SQLITE_TYPES_HPP
