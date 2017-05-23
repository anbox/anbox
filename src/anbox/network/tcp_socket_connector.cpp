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

#include "anbox/network/tcp_socket_connector.h"
#include "anbox/network/connection_context.h"
#include "anbox/network/socket_helper.h"

namespace anbox {
namespace network {
TcpSocketConnector::TcpSocketConnector(
    const boost::asio::ip::address_v4& address, unsigned short port,
    const std::shared_ptr<Runtime>& rt,
    const std::shared_ptr<ConnectionCreator<boost::asio::ip::tcp>>&
        connection_creator)
    : address_(address),
      port_(port),
      runtime_(rt),
      connection_creator_(connection_creator),
      acceptor_(rt->service(), boost::asio::ip::tcp::endpoint(address, port)) {
  start_accept();
}

TcpSocketConnector::~TcpSocketConnector() noexcept { acceptor_.cancel(); }

void TcpSocketConnector::start_accept() {
  auto socket =
      std::make_shared<boost::asio::ip::tcp::socket>(runtime_->service());

  acceptor_.async_accept(*socket,
                         [this, socket](boost::system::error_code const& err) {
                           on_new_connection(socket, err);
                         });
}

void TcpSocketConnector::on_new_connection(
    std::shared_ptr<boost::asio::ip::tcp::socket> const& socket,
    boost::system::error_code const& err) {
  switch (err.value()) {
    case boost::system::errc::success:
      connection_creator_->create_connection_for(socket);
      break;
    default:
      // Socket was closed so don't listen for any further incoming
      // connection attempts.
      return;
  }

  start_accept();
}
}  // namespace network
}  // namespace anbox
