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

#ifndef ANBOX_AUDIO_SERVER_H_
#define ANBOX_AUDIO_SERVER_H_

#include "anbox/runtime.h"
#include "anbox/audio/client_info.h"
#include "anbox/network/socket_messenger.h"
#include "anbox/network/socket_connection.h"
#include "anbox/platform/base_platform.h"

#include <atomic>

namespace anbox {
namespace network {
class PublishedSocketConnector;
} // namespace network
namespace audio {
class Server {
 public:
  Server(const std::shared_ptr<Runtime>& rt, const std::shared_ptr<platform::BasePlatform> &platform);
  ~Server();

  std::string socket_file() const { return socket_file_; }

 private:
  void create_connection_for(std::shared_ptr<boost::asio::basic_stream_socket<
                             boost::asio::local::stream_protocol>> const& socket);

  int next_id();

  std::shared_ptr<platform::BasePlatform> platform_;
  std::string socket_file_;
  std::shared_ptr<network::PublishedSocketConnector> connector_;
  std::shared_ptr<network::Connections<network::SocketConnection>> const connections_;
  std::atomic<int> next_id_;
};
} // namespace audio
} // namespace anbox

#endif
