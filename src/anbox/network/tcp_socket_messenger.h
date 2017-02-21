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

#ifndef ANBOX_NETWORK_TCP_SOCKET_MESSENGER_H_
#define ANBOX_NETWORK_TCP_SOCKET_MESSENGER_H_

#include "anbox/network/base_socket_messenger.h"
#include "anbox/runtime.h"

#include <boost/asio.hpp>

namespace anbox {
namespace network {
class TcpSocketMessenger : public BaseSocketMessenger<boost::asio::ip::tcp> {
 public:
  TcpSocketMessenger(const boost::asio::ip::address_v4 &addr,
                     unsigned short port, const std::shared_ptr<Runtime> &rt);
  TcpSocketMessenger(
      std::shared_ptr<boost::asio::ip::tcp::socket> const &socket);
  ~TcpSocketMessenger();

  unsigned short local_port() const override;

 private:
  unsigned short local_port_;
};
}  // namespace network
}  // namespace anbox

#endif
