/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "anbox/logger.h"

#include "anbox/network/message_receiver.h"
#include "anbox/network/message_sender.h"
#include "anbox/network/socket_connection.h"

#include <boost/signals2.hpp>
#include <boost/throw_exception.hpp>

#include <stdexcept>

#include <sys/socket.h>
#include <sys/types.h>

namespace ba = boost::asio;
namespace bs = boost::system;

namespace anbox {
namespace network {
SocketConnection::SocketConnection(
    std::shared_ptr<MessageReceiver> const& message_receiver,
    std::shared_ptr<MessageSender> const& message_sender, int id_,
    std::shared_ptr<Connections<SocketConnection>> const& connections,
    std::shared_ptr<MessageProcessor> const& processor)
    : message_receiver_(message_receiver),
      message_sender_(message_sender),
      id_(id_),
      connections_(connections),
      processor_(processor) {}

SocketConnection::~SocketConnection() noexcept {}

void SocketConnection::send(char const* data, size_t length) {
  message_sender_->send(data, length);
}

void SocketConnection::read_next_message() {
  auto callback = std::bind(&SocketConnection::on_read_size, this, std::placeholders::_1, std::placeholders::_2);
  message_receiver_->async_receive_msg(callback, ba::buffer(buffer_));
}

void SocketConnection::on_read_size(const boost::system::error_code& error, std::size_t bytes_read) {
  if (error) {
    connections_->remove(id());
    return;
  }

  std::vector<std::uint8_t> data(bytes_read);
  std::copy(buffer_.data(), buffer_.data() + bytes_read, data.data());

  if (processor_->process_data(data))
    read_next_message();
  else
      connections_->remove(id());
}
}  // namespace anbox
}  // namespace network
