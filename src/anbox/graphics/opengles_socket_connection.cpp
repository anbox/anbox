/*
 * Copyright © 2018 The UBports project
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
 * Authored by: Marius Gripsgard <marius@ubports.com>
 */

#include "anbox/logger.h"

#include "anbox/network/message_receiver.h"
#include "anbox/network/message_sender.h"
#include "anbox/graphics/opengles_socket_connection.h"
#include "anbox/graphics/buffered_io_stream.h"

#include <boost/signals2.hpp>
#include <boost/throw_exception.hpp>

#include <stdexcept>

#include <sys/socket.h>
#include <sys/types.h>

namespace ba = boost::asio;
namespace bs = boost::system;

namespace anbox {
namespace graphics {

OpenGlesSocketConnection::OpenGlesSocketConnection(
      std::shared_ptr<network::MessageReceiver> const& message_receiver,
      std::shared_ptr<network::MessageSender> const& message_sender, int id,
      std::shared_ptr<network::Connections<network::SocketConnection>> const& connections,
      std::shared_ptr<network::MessageProcessor> const& processor):
      SocketConnection(message_receiver,
                      message_sender, id,
                      connections, processor) {}

void OpenGlesSocketConnection::read_next_message() {
  auto callback = std::bind(&OpenGlesSocketConnection::on_read_size, this, std::placeholders::_1, std::placeholders::_2);
  message_receiver_->async_receive_msg(callback, ba::buffer(buffer_));
}

void OpenGlesSocketConnection::on_read_size(const boost::system::error_code& error, std::size_t bytes_read) {
  if (error) {
    connections_->remove(id());
    return;
  }

  Buffer data{};
  data.resize_noinit(bytes_read);

  // Use memcpy here, its faster then std::copy
  memcpy(data.data(), buffer_.data(), bytes_read);

  if (processor_->process_data(std::move(data)))
    read_next_message();
  else
      connections_->remove(id());
}
}  // namespace anbox
}  // namespace graphics
