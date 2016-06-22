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

#include "anbox/logger.h"
#include "anbox/graphics/opengles_message_processor.h"
#include "anbox/network/socket_messenger.h"
#include "anbox/network/connections.h"
#include "anbox/network/delegate_message_processor.h"

namespace anbox {
namespace graphics {

OpenGlesMessageProcessor::OpenGlesMessageProcessor(const std::string &renderer_socket_path,
                                                   const std::shared_ptr<Runtime> &rt,
                                                   const std::shared_ptr<network::SocketMessenger> &messenger) :
    client_messenger_(messenger) {

    connect_and_attach(renderer_socket_path, rt);
}

OpenGlesMessageProcessor::~OpenGlesMessageProcessor() {
}

void OpenGlesMessageProcessor::connect_and_attach(const std::string &socket_path,
                                                  const std::shared_ptr<Runtime> &rt) {

    auto socket = std::make_shared<boost::asio::local::stream_protocol::socket>(rt->service());
    socket->connect(boost::asio::local::stream_protocol::endpoint(socket_path));

    messenger_ = std::make_shared<network::SocketMessenger>(socket);
    renderer_ = std::make_shared<network::SocketConnection>(
                    messenger_,
                    messenger_,
                    0,
                    std::make_shared<network::Connections<network::SocketConnection>>(),
                    std::make_shared<network::DelegateMessageProcessor>([&](const std::vector<std::uint8_t> &data) {
        client_messenger_->send(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    }));

    renderer_->read_next_message();
}

bool OpenGlesMessageProcessor::process_data(const std::vector<std::uint8_t> &data) {
    messenger_->send(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}


} // namespace graphics
} // namespace anbox
