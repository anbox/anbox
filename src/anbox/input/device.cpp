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

#include "anbox/input/device.h"
#include "anbox/logger.h"
#include "anbox/network/delegate_connection_creator.h"
#include "anbox/network/delegate_message_processor.h"
#include "anbox/network/local_socket_messenger.h"
#include "anbox/qemu/null_message_processor.h"

#include <time.h>

namespace anbox {
namespace input {
std::shared_ptr<Device> Device::create(
    const std::string &path, const std::shared_ptr<Runtime> &runtime) {
  auto sp = std::make_shared<Device>();

  auto delegate_connector = std::make_shared<
      network::DelegateConnectionCreator<boost::asio::local::stream_protocol>>(
      [sp](std::shared_ptr<boost::asio::local::stream_protocol::socket> const
               &socket) { sp->new_client(socket); });

  sp->connector_ = std::make_shared<network::PublishedSocketConnector>(
      path, runtime, delegate_connector);

  // The socket is created with user permissions (e.g. rwx------),
  // which prevents the container from accessing it. Make sure it is writable.
  ::chmod(path.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

  return sp;
}

Device::Device()
    : next_connection_id_(0),
      connections_(
          std::make_shared<network::Connections<network::SocketConnection>>()) {
  ::memset(&info_, 0, sizeof(info_));
}

Device::~Device() {}

void Device::send_events(const std::vector<Event> &events) {
  struct CompatEvent {
    // NOTE: A bit dirty but as we're running currently a 64 bit container
    // struct input_event has a different size. We rebuild the struct here
    // to reach the correct size.
    std::uint64_t sec;
    std::uint64_t usec;
    std::uint16_t type;
    std::uint16_t code;
    std::uint32_t value;
  };

  struct timespec spec;
  clock_gettime(CLOCK_MONOTONIC, &spec);

  auto data = new CompatEvent[events.size()];
  int n = 0;
  for (const auto &event : events) {
    data[n].sec = spec.tv_sec;
    data[n].usec = spec.tv_nsec / 1000;
    data[n].type = event.type;
    data[n].code = event.code;
    data[n].value = event.value;
    n++;
  }

  for (unsigned n = 0; n < connections_->size(); n++) {
    connections_->at(n)->send(reinterpret_cast<const char *>(data),
                              events.size() * sizeof(struct CompatEvent));
  }
}

void Device::set_name(const std::string &name) {
  snprintf(info_.name, 80, "%s", name.c_str());
}

void Device::set_driver_version(const int &version) {
  info_.driver_version = version;
}

void Device::set_input_id(const struct input_id &id) {
  info_.id.bustype = id.bustype;
  info_.id.product = id.product;
  info_.id.vendor = id.vendor;
  info_.id.version = id.version;
}

void Device::set_physical_location(const std::string &physical_location) {
  snprintf(info_.physical_location, 80, "%s", physical_location.c_str());
}

void Device::set_key_bit(const std::uint64_t &bit) {
  set_bit(info_.key_bitmask, bit);
}

void Device::set_abs_bit(const std::uint64_t &bit) {
  set_bit(info_.abs_bitmask, bit);
}

void Device::set_rel_bit(const std::uint64_t &bit) {
  set_bit(info_.rel_bitmask, bit);
}

void Device::set_sw_bit(const std::uint64_t &bit) {
  set_bit(info_.sw_bitmask, bit);
}

void Device::set_led_bit(const std::uint64_t &bit) {
  set_bit(info_.led_bitmask, bit);
}

void Device::set_ff_bit(const std::uint64_t &bit) {
  set_bit(info_.ff_bitmask, bit);
}

void Device::set_prop_bit(const std::uint64_t &bit) {
  set_bit(info_.prop_bitmask, bit);
}

void Device::set_abs_min(const std::uint64_t &bit, const std::uint32_t &value) {
  info_.abs_min[bit] = value;
}

void Device::set_abs_max(const std::uint64_t &bit, const std::uint32_t &value) {
  info_.abs_max[bit] = value;
}

void Device::set_bit(std::uint8_t *array, const std::uint64_t &bit) {
  array[bit / 8] |= (1 << (bit % 8));
}

void Device::set_unique_id(const std::string &unique_id) {
  snprintf(info_.unique_id, 80, "%s", unique_id.c_str());
}

std::string Device::socket_path() const { return connector_->socket_file(); }

int Device::next_id() { return next_connection_id_++; }

void Device::new_client(
    std::shared_ptr<boost::asio::local::stream_protocol::socket> const
        &socket) {
  auto const messenger =
      std::make_shared<network::LocalSocketMessenger>(socket);
  auto const &connection = std::make_shared<network::SocketConnection>(
      messenger, messenger, next_id(), connections_,
      std::make_shared<qemu::NullMessageProcessor>());
  connection->set_name("input-device");
  connections_->add(connection);

  // Send all necessary information about our device so that the remote
  // side can properly configure itself for this input device
  connection->send(reinterpret_cast<char const *>(&info_), sizeof(info_));
}
}  // namespace input
}  // namespace anbox
