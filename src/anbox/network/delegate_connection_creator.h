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

#ifndef ANBOX_NETWORK_DELEGATE_CONNECTION_CREATOR_H_
#define ANBOX_NETWORK_DELEGATE_CONNECTION_CREATOR_H_

#include <boost/asio.hpp>
#include "anbox/network/connection_creator.h"

#include <functional>

namespace anbox {
namespace network {
template <typename stream_protocol>
class DelegateConnectionCreator : public ConnectionCreator<stream_protocol> {
 public:
  DelegateConnectionCreator(
      std::function<void(std::shared_ptr<boost::asio::basic_stream_socket<
                             stream_protocol>> const&)>
          delegate)
      : delegate_(delegate) {}

  void create_connection_for(
      std::shared_ptr<boost::asio::basic_stream_socket<stream_protocol>> const&
          socket) override {
    if (delegate_)
      delegate_(socket);
    else
      socket->close();
  }

 private:
  std::function<void(std::shared_ptr<
                     boost::asio::basic_stream_socket<stream_protocol>> const&)>
      delegate_;
};
}  // namespace network
}  // namespace anbox

#endif
