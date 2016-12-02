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

#include "anbox/network/tcp_socket_messenger.h"

namespace anbox {
namespace network {
TcpSocketMessenger::TcpSocketMessenger(const boost::asio::ip::address_v4 &addr,
                                       unsigned short port,
                                       const std::shared_ptr<Runtime> &rt) {
  boost::asio::ip::tcp::endpoint endpoint(addr, port);
  auto socket = std::make_shared<boost::asio::ip::tcp::socket>(rt->service());
  socket->connect(endpoint);
  setup(socket);
  local_port_ = socket->local_endpoint().port();
}

TcpSocketMessenger::TcpSocketMessenger(
    std::shared_ptr<boost::asio::ip::tcp::socket> const &socket)
    : BaseSocketMessenger(socket) {}

TcpSocketMessenger::~TcpSocketMessenger() {}

unsigned short TcpSocketMessenger::local_port() const { return local_port_; }
}  // namespace network
}  // namespace anbox
