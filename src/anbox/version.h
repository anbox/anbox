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

#ifndef ANBOX_VERSION_H_
#define ANBOX_VERSION_H_

#include <cstdint>

namespace anbox {
namespace build {
/// @brief version_major marks the major version of the library. The constant is
/// meant to be used
/// by client code both at build and runtime, enabling version checks.
static constexpr const std::uint32_t version_major{1};
/// @brief version_major marks the minor version of the library. The constant is
/// meant to be used
/// by client code both at build and runtime, enabling version checks.
static constexpr const std::uint32_t version_minor{0};
/// @brief version_patch marks the major version of the library. The constant is
/// meant to be used
/// by client code both at build and runtime, enabling version checks.
static constexpr const std::uint32_t version_patch{1};
}  // namespace build

/// @brief version queries the version of the library, placing the result in
/// major, minor and patch.
void version(std::uint32_t& major, std::uint32_t& minor, std::uint32_t& patch);
}  // namespace build

#endif  // ANBOX_VERSION_H_
