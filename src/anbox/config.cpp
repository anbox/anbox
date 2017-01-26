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


#include "anbox/config.h"
#include "anbox/utils.h"


#include <cstring>

namespace fs = boost::filesystem;

namespace {
static std::string runtime_dir() {
  static std::string path;
  if (path.empty()) {
    path = anbox::utils::get_env_value("XDG_RUNTIME_DIR", "");
    if (path.empty())
      BOOST_THROW_EXCEPTION(std::runtime_error("No runtime directory specified"));
  }
  return path;
}
}

void anbox::SystemConfiguration::set_data_path(const std::string &path) {
  data_path = path;
}

std::string anbox::SystemConfiguration::rootfs_dir() const {
  return (data_path / "rootfs").string();
}

std::string anbox::SystemConfiguration::log_dir() const {
  return (data_path / "logs").string();
}

std::string anbox::SystemConfiguration::container_config_dir() const {
  return (data_path / "containers").string();
}

std::string anbox::SystemConfiguration::container_socket_path() const {
  return "/run/anbox-container.socket";
}

std::string anbox::SystemConfiguration::socket_dir() const {
  static std::string dir = anbox::utils::string_format("%s/anbox/sockets", runtime_dir());
  return dir;
}

std::string anbox::SystemConfiguration::input_device_dir() const {
  static std::string dir = anbox::utils::string_format("%s/anbox/input", runtime_dir());
  return dir;
}

anbox::SystemConfiguration& anbox::SystemConfiguration::instance() {
  static SystemConfiguration config;
  return config;
}
