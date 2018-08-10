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

#ifndef ANBOX_SYSTEM_CONFIGURATION_H_
#define ANBOX_SYSTEM_CONFIGURATION_H_

#include <string>
#include <memory>

#include <boost/filesystem.hpp>

namespace anbox {
class SystemConfiguration {
 public:
  static SystemConfiguration& instance();

  virtual ~SystemConfiguration() = default;

  void set_data_path(const std::string &path);

  boost::filesystem::path data_dir() const;
  std::string rootfs_dir() const;
  std::string overlay_dir() const;
  std::string combined_rootfs_dir() const;
  std::string log_dir() const;
  std::string socket_dir() const;
  std::string container_config_dir() const;
  std::string container_socket_path() const;
  std::string container_devices_dir() const;
  std::string input_device_dir() const;
  std::string application_item_dir() const;
  std::string resource_dir() const;

 protected:
  SystemConfiguration();

  boost::filesystem::path data_path;
  boost::filesystem::path resource_path;
};
}  // namespace anbox

#endif // ANBOX_SYSTEM_CONFIGURATION_H_
