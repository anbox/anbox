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

#ifndef ANBOX_GRAPHICS_OPENGLES_SOCKET_CONNECTION_H_
#define ANBOX_GRAPHICS_OPENGLES_SOCKET_CONNECTION_H_

#include "anbox/network/connections.h"
#include "anbox/network/message_processor.h"
#include "anbox/network/message_receiver.h"
#include "anbox/network/message_sender.h"
#include "anbox/network/socket_connection.h"
#include "anbox/graphics/opengles_message_processor.h"

#include <boost/asio.hpp>

#include <sys/types.h>

namespace anbox {
namespace graphics {
class OpenGlesSocketConnection : public network::SocketConnection {
public:
  OpenGlesSocketConnection(
      std::shared_ptr<network::MessageReceiver> const& message_receiver,
      std::shared_ptr<network::MessageSender> const& message_sender, int id,
      std::shared_ptr<network::Connections<network::SocketConnection>> const& connections,
      std::shared_ptr<network::MessageProcessor> const& processor);
  void read_next_message() override;
private:
  void on_read_size(const boost::system::error_code& ec,
                        std::size_t bytes_read) override;
};
}  // namespace anbox
}  // namespace graphics

#endif
