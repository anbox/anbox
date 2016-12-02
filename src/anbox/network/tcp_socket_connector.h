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

#ifndef ANBOX_TCPSOCKETCONNECTOR_H
#define ANBOX_TCPSOCKETCONNECTOR_H

#include <boost/asio/ip/tcp.hpp>

#include "anbox/do_not_copy_or_move.h"
#include "anbox/runtime.h"

#include "anbox/network/connection_creator.h"
#include "anbox/network/connector.h"

namespace anbox {
namespace network {
class TcpSocketConnector : public DoNotCopyOrMove, public Connector {
 public:
  explicit TcpSocketConnector(
      const boost::asio::ip::address_v4 &address, unsigned short port,
      const std::shared_ptr<Runtime> &rt,
      const std::shared_ptr<ConnectionCreator<boost::asio::ip::tcp>>
          &connection_creator);
  ~TcpSocketConnector() noexcept;

  unsigned short port() const { return port_; }

 private:
  void start_accept();
  void on_new_connection(
      std::shared_ptr<boost::asio::ip::tcp::socket> const &socket,
      boost::system::error_code const &err);

  boost::asio::ip::address_v4 address_;
  unsigned short port_;
  std::shared_ptr<Runtime> runtime_;
  std::shared_ptr<ConnectionCreator<boost::asio::ip::tcp>> connection_creator_;
  boost::asio::ip::tcp::acceptor acceptor_;
};
}  // namespace network
}  // namespace anbox

#endif
