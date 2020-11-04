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

#ifndef ANBOX_NETWORK_QEMU_PIPE_CONNECTION_CREATOR_H_
#define ANBOX_NETWORK_QEMU_PIPE_CONNECTION_CREATOR_H_

#include <boost/asio.hpp>

#include <memory>

#include "anbox/application/sensors_state.h"
#include "anbox/application/gps_info_broker.h"
#include "anbox/do_not_copy_or_move.h"
#include "anbox/network/connection_creator.h"
#include "anbox/network/connections.h"
#include "anbox/network/socket_connection.h"
#include "anbox/network/socket_messenger.h"
#include "anbox/runtime.h"

class Renderer;

namespace anbox::qemu {
class PipeConnectionCreator
    : public network::ConnectionCreator<boost::asio::local::stream_protocol> {
 public:
  PipeConnectionCreator(std::shared_ptr<Renderer> renderer, std::shared_ptr<Runtime> rt, std::shared_ptr<anbox::application::SensorsState> ss, std::shared_ptr<anbox::application::GpsInfoBroker> gpsInfoBroker);
  ~PipeConnectionCreator() noexcept;

  void create_connection_for(
      std::shared_ptr<boost::asio::basic_stream_socket<
          boost::asio::local::stream_protocol>> const &socket) override;

  enum class client_type {
    invalid,
    opengles,
    qemud_boot_properties,
    qemud_hw_control,
    qemud_sensors,
    qemud_camera,
    qemud_fingerprint,
    qemud_gps,
    qemud_gsm,
    qemud_adb,
    bootanimation,
  };

 private:
  int next_id();

  client_type identify_client(
      std::shared_ptr<network::SocketMessenger> const &messenger);
  std::shared_ptr<network::MessageProcessor> create_processor(
      const client_type &type,
      const std::shared_ptr<network::SocketMessenger> &messenger);

  std::shared_ptr<Renderer> renderer_;
  std::shared_ptr<Runtime> runtime_;
  std::shared_ptr<application::SensorsState> sensors_state_;
  std::shared_ptr<application::GpsInfoBroker> gps_info_broker_;
  std::atomic<int> next_connection_id_;
  std::shared_ptr<network::Connections<network::SocketConnection>> const connections_;
};
}
#endif
