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

#ifndef ANBOX_RPC_CONNECTION_CREATOR_H_
#define ANBOX_RPC_CONNECTION_CREATOR_H_

#include <boost/asio.hpp>

#include <memory>

#include "anbox/do_not_copy_or_move.h"
#include "anbox/network/connection_creator.h"
#include "anbox/network/connections.h"
#include "anbox/network/socket_connection.h"
#include "anbox/network/socket_messenger.h"
#include "anbox/runtime.h"

namespace anbox {
namespace rpc {
class ConnectionCreator
    : public network::ConnectionCreator<boost::asio::local::stream_protocol> {
 public:
  typedef std::function<std::shared_ptr<network::MessageProcessor>(
      const std::shared_ptr<network::MessageSender> &)>
      MessageProcessorFactory;

  ConnectionCreator(const std::shared_ptr<Runtime> &rt,
                    const MessageProcessorFactory &factory);
  ~ConnectionCreator() noexcept;

  void create_connection_for(
      std::shared_ptr<boost::asio::basic_stream_socket<
          boost::asio::local::stream_protocol>> const &socket) override;

 private:
  int next_id();

  std::shared_ptr<Runtime> runtime_;
  std::atomic<int> next_connection_id_;
  std::shared_ptr<network::Connections<network::SocketConnection>> const
      connections_;
  MessageProcessorFactory message_processor_factory_;
};
}  // namespace rpc
}  // namespace anbox

#endif
