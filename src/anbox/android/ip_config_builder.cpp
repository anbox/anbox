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

#include "anbox/android/ip_config_builder.h"

#include <boost/endian/buffers.hpp>

#include <ostream>
#include <sstream>

namespace {
constexpr const char *assignment_key{"ipAssignment"};
constexpr const char *link_address_key{"linkAddress"};
constexpr const char *gateway_key{"gateway"};
constexpr const char *dns_key{"dns"};
constexpr const char *id_key{"id"};
constexpr const char *eos_key{"eos"};
constexpr const char *assignment_static{"STATIC"};
constexpr const char *assignment_dhcp{"DHCP"};
constexpr const char *assignment_unknown{"UNKNOWN"};

constexpr const std::uint32_t is_default_gateway{0};
constexpr const std::uint32_t gateway_is_present{1};

namespace aa = anbox::android;
std::string assignment_to_string(const aa::IpConfigBuilder::Assignment &value) {
  switch (value) {
  case anbox::android::IpConfigBuilder::Assignment::Static:
    return assignment_static;
    break;
  case anbox::android::IpConfigBuilder::Assignment::DHCP:
    return assignment_dhcp;
    break;
  default:
    break;
  }
  return assignment_unknown;
}
}

namespace anbox {
namespace android {
std::size_t IpConfigBuilder::write(common::BinaryWriter &writer) {
  writer.set_byte_order(common::BinaryWriter::Order::Big);

  // See http://androidxref.com/7.1.1_r6/xref/frameworks/base/services/core/java/com/android/server/net/IpConfigStore.java
  // for more details on the binary file format used here.

  writer.write_uint32(static_cast<std::uint32_t>(version_));

  writer.write_string_with_size(assignment_key);
  writer.write_string_with_size(assignment_to_string(assignment_));

  writer.write_string_with_size(link_address_key);
  writer.write_string_with_size(link_.address);
  writer.write_uint32(link_.prefix_length);

  writer.write_string_with_size(gateway_key);
  writer.write_uint32(is_default_gateway);
  writer.write_uint32(gateway_is_present);
  writer.write_string_with_size(gateway_);

  writer.write_string_with_size(dns_key);
  for (const auto &server : dns_servers_)
    writer.write_string_with_size(server);

  writer.write_string_with_size(id_key);
  writer.write_uint32(id_);

  writer.write_string_with_size(eos_key);

  return writer.bytes_written();
}

void IpConfigBuilder::set_version(const Version &version) {
  version_ = version;
}

void IpConfigBuilder::set_assignment(const Assignment &assignment) {
  assignment_ = assignment;
}

void IpConfigBuilder::set_link_address(const std::string &address, uint32_t prefix_length) {
  link_.address = address;
  link_.prefix_length = prefix_length;
}

void IpConfigBuilder::set_gateway(const std::string &gateway) {
  gateway_ = gateway;
}

void IpConfigBuilder::set_dns_servers(const std::vector<std::string> &dns_servers) {
  dns_servers_ = dns_servers;
}

void IpConfigBuilder::set_id(uint32_t id) {
  id_ = id;
}
}  // namespace android
}  // namespace anbox
