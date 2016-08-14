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

#include "anbox/network/socket_messenger.h"
#include "anbox/network/message_sender.h"
#include "anbox/network/message_receiver.h"
#include "anbox/runtime.h"

#include <boost/asio/local/stream_protocol.hpp>

namespace anbox {
namespace network {
class LocalSocketMessenger : public MessageSender,
                             public MessageReceiver {
public:
    LocalSocketMessenger(const std::string &path, const std::shared_ptr<Runtime> &rt);
    ~LocalSocketMessenger();

    void send(char const* data, size_t length) override;
    void async_receive_msg(AnboxReadHandler const& handle, boost::asio::mutable_buffers_1 const &buffer) override;
    boost::system::error_code receive_msg(boost::asio::mutable_buffers_1 const& buffer) override;
    size_t available_bytes() override;

private:
    std::shared_ptr<boost::asio::local::stream_protocol::socket> socket_;
    std::shared_ptr<SocketMessenger> messenger_;
};
} // namespace network
} // namespace anbox

#endif
