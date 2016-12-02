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

#include <cstring>

#include "anbox/config.h"
#include "anbox/utils.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace anbox {
namespace config {
std::string in_snap_dir(const std::string &path) {
  return utils::prefix_dir_from_env(path, "SNAP");
}

std::string in_snap_data_dir(const std::string &path) {
  return utils::prefix_dir_from_env(path, "SNAP_COMMON");
}

std::string in_snap_user_data_dir(const std::string &path) {
  return utils::prefix_dir_from_env(path, "SNAP_USER_COMMON");
}

std::string home_dir() {
  static std::string path;
  if (path.empty()) {
    path = utils::get_env_value("HOME", "");
    if (path.empty())
      BOOST_THROW_EXCEPTION(std::runtime_error("No home directory specified"));
  }
  return path;
}

std::string runtime_dir() {
  static std::string path;
  if (path.empty()) {
    path = utils::get_env_value("XDG_RUNTIME_DIR", "");
    if (path.empty())
      BOOST_THROW_EXCEPTION(
          std::runtime_error("No runtime directory specified"));
  }
  return path;
}

std::string state_dir() {
  static std::string path = "/var/lib";
  return path;
}

std::string log_path() {
  static std::string path =
      in_snap_data_dir(utils::string_format("%s/anbox/", state_dir()));
  return path;
}

std::string socket_path() {
  static std::string path =
      utils::string_format("%s/anbox/sockets", runtime_dir());
  return path;
}

std::string data_path() {
  static std::string path = utils::string_format("%s/.anbox/data", home_dir());
  return path;
}

std::string rootfs_path() {
  static std::string path =
      in_snap_data_dir(utils::string_format("%s/anbox/rootfs", state_dir()));
  return path;
}

std::string container_config_path() {
  static std::string path = in_snap_data_dir(
      utils::string_format("%s/anbox/containers", state_dir()));
  return path;
}

std::string container_socket_path() {
  std::string path = "/run/anbox-container.socket";
  return path;
}

std::string host_input_device_path() {
  static std::string path =
      utils::string_format("%s/anbox/input-devices", runtime_dir());
  return path;
}
}  // namespace config
}  // namespace anbox
