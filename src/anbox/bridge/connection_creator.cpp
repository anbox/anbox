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


#include "anbox/bridge/connection_creator.h"
#include "anbox/bridge/message_processor.h"
#include "anbox/network/socket_messenger.h"
#include "anbox/logger.h"

#include <string>

namespace ba = boost::asio;

namespace anbox {
namespace bridge {
ConnectionCreator::ConnectionCreator(const std::shared_ptr<Runtime> &rt, const MessageProcessorFactory &factory) :
    runtime_(rt),
    next_connection_id_(0),
    connections_(std::make_shared<network::Connections<network::SocketConnection>>()),
    message_processor_factory_(factory) {
}

ConnectionCreator::~ConnectionCreator() {
}

void ConnectionCreator::create_connection_for(
        std::shared_ptr<boost::asio::local::stream_protocol::socket> const& socket) {

    if (connections_->size() >= 1) {
        socket->close();
        WARNING("A second client tried to connect. Denied request as we already have one"
                "and only allow a single client");
        return;
    }

    DEBUG("");

    auto const messenger = std::make_shared<network::SocketMessenger>(socket);
    auto const processor = message_processor_factory_(messenger);

    auto const& connection = std::make_shared<network::SocketConnection>(
                messenger, messenger, next_id(), connections_, processor);
    connections_->add(connection);
    connection->read_next_message();
}

int ConnectionCreator::next_id()
{
    return next_connection_id_.fetch_add(1);
}

} // namespace anbox
} // namespace network
