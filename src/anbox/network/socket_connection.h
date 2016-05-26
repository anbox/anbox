/*
 * Copyright © 2012-2014 Canonical Ltd.
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

#ifndef ANBOX_NETWORK_SOCKET_CONNECTION_H_
#define ANBOX_NETWORK_SOCKET_CONNECTION_H_

#include "anbox/network/connections.h"
#include "anbox/network/message_receiver.h"
#include "anbox/network/message_processor.h"

#include <boost/asio.hpp>

#include <sys/types.h>

namespace anbox {
namespace network {
class SocketConnection
{
public:
    SocketConnection(
        std::shared_ptr<MessageReceiver> const& message_receiver,
        int id,
        std::shared_ptr<Connections<SocketConnection>> const& connections,
        std::shared_ptr<MessageProcessor> const& processor);

    ~SocketConnection() noexcept;

    int id() const { return id_; }

    void read_next_message();

private:
    void on_response_sent(boost::system::error_code const& error, std::size_t);
    void on_read_size(const boost::system::error_code& ec, std::size_t bytes_read);

    std::shared_ptr<MessageReceiver> const message_receiver_;
    int const id_;
    std::shared_ptr<Connections<SocketConnection>> const connections_;
    std::shared_ptr<MessageProcessor> const processor_;
    std::array<std::uint8_t, 8192> buffer_;
};
} // namespace anbox
} // namespace network

#endif
