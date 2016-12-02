/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
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

#ifndef ANBOX_NETWORK_SOCKET_HELPER_H_
#define ANBOX_NETWORK_SOCKET_HELPER_H_

#include <string>

namespace anbox {
namespace network {
bool socket_file_exists(std::string const& filename);
bool socket_exists(std::string const& socket_name);
std::string remove_socket_if_stale(std::string const& socket_name);
}  // namespace network
}  // namespace anbox

#endif
