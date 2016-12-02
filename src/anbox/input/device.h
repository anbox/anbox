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

#ifndef ANBOX_INPUT_DEVICE_H_
#define ANBOX_INPUT_DEVICE_H_

#include "anbox/network/connections.h"
#include "anbox/network/published_socket_connector.h"
#include "anbox/network/socket_connection.h"
#include "anbox/runtime.h"

#include <vector>

#include <linux/input.h>

namespace anbox {
namespace input {
struct Event {
  std::uint16_t type;
  std::uint16_t code;
  std::int32_t value;
};

class Device : public std::enable_shared_from_this<Device> {
 public:
  static std::shared_ptr<Device> create(
      const std::string &path, const std::shared_ptr<Runtime> &runtime);

  Device();
  ~Device();

  void send_events(const std::vector<Event> &events);
  void send_event(const std::uint16_t &code, const std::uint16_t &event,
                  const std::int32_t &value);

  void set_name(const std::string &name);
  void set_driver_version(const int &version);
  void set_input_id(const struct input_id &id);
  void set_physical_location(const std::string &physical_location);
  void set_unique_id(const std::string &unique_id);
  void set_key_bit(const std::uint64_t &bit);
  void set_abs_bit(const std::uint64_t &bit);
  void set_rel_bit(const std::uint64_t &bit);
  void set_sw_bit(const std::uint64_t &bit);
  void set_led_bit(const std::uint64_t &bit);
  void set_ff_bit(const std::uint64_t &bit);
  void set_prop_bit(const std::uint64_t &bit);

  void set_abs_min(const std::uint64_t &bit, const std::uint32_t &value);
  void set_abs_max(const std::uint64_t &bit, const std::uint32_t &value);

  std::string socket_path() const;

 private:
  int next_id();
  void new_client(std::shared_ptr<
                  boost::asio::local::stream_protocol::socket> const &socket);

  // NOTE: If you modify this struct you have to modify the version on
  // the Android side too. See
  // frameworks/native/services/inputflinger/EventHub.cpp
  struct Info {
    char name[80];
    int driver_version;
    struct input_id id;
    char physical_location[80];
    char unique_id[80];
    std::uint8_t key_bitmask[(KEY_MAX + 1) / 8];
    std::uint8_t abs_bitmask[(ABS_MAX + 1) / 8];
    std::uint8_t rel_bitmask[(REL_MAX + 1) / 8];
    std::uint8_t sw_bitmask[(SW_MAX + 1) / 8];
    std::uint8_t led_bitmask[(LED_MAX + 1) / 8];
    std::uint8_t ff_bitmask[(FF_MAX + 1) / 8];
    std::uint8_t prop_bitmask[(INPUT_PROP_MAX + 1) / 8];
    std::uint32_t abs_max[ABS_CNT];
    std::uint32_t abs_min[ABS_CNT];
  };

  void set_bit(std::uint8_t *array, const std::uint64_t &bit);

  std::shared_ptr<network::PublishedSocketConnector> connector_;
  std::atomic<int> next_connection_id_;
  std::shared_ptr<network::Connections<network::SocketConnection>> connections_;
  Info info_;
};
}  // namespace input
}  // namespace anbox

#endif
