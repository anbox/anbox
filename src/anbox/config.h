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

#ifndef ANBOX_CONFIG_H_
#define ANBOX_CONFIG_H_

#include <string>

namespace anbox {
namespace config {
std::string in_snap_dir(const std::string &path);
std::string in_snap_data_dir(const std::string &path);
std::string in_snap_user_data_dir(const std::string &path);
std::string data_path();
std::string rootfs_path();
std::string log_path();
std::string socket_path();
std::string container_config_path();
std::string container_socket_path();
std::string host_input_device_path();
}  // namespace config
}  // namespace anbox

#endif
