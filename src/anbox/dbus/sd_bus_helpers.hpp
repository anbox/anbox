/*
 * Copyright (C) 2019 Martin Unzner <martin.u@posteo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ANBOX_DBUS_SD_BUS_HELPERS_HPP_
#define ANBOX_DBUS_SD_BUS_HELPERS_HPP_

namespace anbox {
namespace dbus {

extern "C" {
#include "sd_bus_helpers.h"
}
}  // namespace dbus
}  // namespace anbox

#endif
