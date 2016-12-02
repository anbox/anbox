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

#ifndef ANBOX_NETWORK_LOCAL_SOCKET_MESSENGER_H_
#define ANBOX_NETWORK_LOCAL_SOCKET_MESSENGER_H_

#include "anbox/network/base_socket_messenger.h"
#include "anbox/runtime.h"

#include <boost/asio/local/stream_protocol.hpp>

namespace anbox {
namespace network {
class LocalSocketMessenger
    : public BaseSocketMessenger<boost::asio::local::stream_protocol> {
 public:
  LocalSocketMessenger(
      std::shared_ptr<boost::asio::local::stream_protocol::socket> const
          &socket);
  LocalSocketMessenger(const std::string &path,
                       const std::shared_ptr<Runtime> &rt);
  ~LocalSocketMessenger();

 private:
  std::shared_ptr<boost::asio::local::stream_protocol::socket> socket_;
};
}  // namespace network
}  // namespace anbox

#endif
