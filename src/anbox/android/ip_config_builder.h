/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
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

#ifndef ANBOX_ANDROID_IPCONFIGBUILDER_H_
#define ANBOX_ANDROID_IPCONFIGBUILDER_H_

#include "anbox/common/binary_writer.h"

#include <string>
#include <vector>
#include <cstdint>

namespace anbox {
namespace android {
class IpConfigBuilder {
 public:
  enum class Version : std::uint32_t {
    Version1 = 1,
    Version2 = 2,
  };

  enum class Assignment {
    Static,
    DHCP,
  };

  IpConfigBuilder() = default;

  std::size_t write(common::BinaryWriter &writer);

  void set_version(const Version &version);
  void set_assignment(const Assignment &assignment);
  void set_link_address(const std::string &address, std::uint32_t prefix_length);
  void set_gateway(const std::string &gateway);
  void set_dns_servers(const std::vector<std::string> &dns_servers);
  void set_id(std::uint32_t id);

 private:
  Version version_;
  Assignment assignment_;
  struct {
    std::string address;
    std::uint32_t prefix_length;
  } link_;
  std::string gateway_;
  std::vector<std::string> dns_servers_;
  std::uint32_t id_;
};
}  // namespace android
}  // namespace anbox

#endif
