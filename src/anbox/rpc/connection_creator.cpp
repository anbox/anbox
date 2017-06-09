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

#include "anbox/rpc/connection_creator.h"
#include "anbox/logger.h"
#include "anbox/network/local_socket_messenger.h"
#include "anbox/rpc/message_processor.h"

#include <string>

namespace ba = boost::asio;

namespace anbox {
namespace rpc {
ConnectionCreator::ConnectionCreator(const std::shared_ptr<Runtime>& rt,
                                     const MessageProcessorFactory& factory)
    : runtime_(rt),
      next_connection_id_(0),
      connections_(
          std::make_shared<network::Connections<network::SocketConnection>>()),
      message_processor_factory_(factory) {}

ConnectionCreator::~ConnectionCreator() noexcept {}

void ConnectionCreator::create_connection_for(
    std::shared_ptr<boost::asio::local::stream_protocol::socket> const&
        socket) {
  if (connections_->size() >= 1) {
    socket->close();
    WARNING(
        "A second client tried to connect. Denied request as we already have "
        "one and only allow a single client");
    return;
  }

  auto const messenger =
      std::make_shared<network::LocalSocketMessenger>(socket);
  auto const processor = message_processor_factory_(messenger);

  auto const& connection = std::make_shared<network::SocketConnection>(
      messenger, messenger, next_id(), connections_, processor);
  connection->set_name("rpc");
  connections_->add(connection);
  connection->read_next_message();
}

int ConnectionCreator::next_id() { return next_connection_id_.fetch_add(1); }

}  // namespace rpc
}  // namespace anbox
