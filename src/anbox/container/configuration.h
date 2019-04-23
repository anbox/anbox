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

#ifndef ANBOX_CONTAINER_CONFIGURATION_H_
#define ANBOX_CONTAINER_CONFIGURATION_H_

#include <string>
#include <vector>
#include <unordered_map>

namespace anbox {
namespace container {
struct DeviceSpecification {
  uint32_t permission;
  std::string old_device_name = "";
};

struct Configuration {
  std::unordered_map<std::string, std::string> bind_mounts;
  std::unordered_map<std::string, DeviceSpecification> devices;
  std::vector<std::string> extra_properties;
};
}  // namespace container
}  // namespace anbox

#endif
