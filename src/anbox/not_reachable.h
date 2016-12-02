/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 *
 */

#ifndef ANBOX_UTIL_NOT_REACHABLE_H_
#define ANBOX_UTIL_NOT_REACHABLE_H_

#include <stdexcept>
#include <string>

namespace anbox {
namespace util {
/// @brief NotReachable is thrown from not_reachable.
struct NotReachable : public std::logic_error {
  /// @brief NotImplemented initializes a new instance for the given function
  /// name.
  NotReachable(const std::string& function, const std::string& file,
               std::uint32_t line);
};

/// @brief not_reachable throws NotReachable.
[[noreturn]] void not_reachable(const std::string& function,
                                const std::string& file, std::uint32_t line);
}  // namespace util
}  // namespace anbox

#endif
