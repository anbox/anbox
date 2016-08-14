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

#include "anbox/network/local_socket_messenger.h"
#include "anbox/network/socket_helper.h"
#include "anbox/utils.h"

#include <boost/system/error_code.hpp>

namespace anbox {
namespace network {
LocalSocketMessenger::LocalSocketMessenger(const std::string &path,
                                           const std::shared_ptr<Runtime> &rt) :
    socket_(std::make_shared<boost::asio::local::stream_protocol::socket>(rt->service())) {

    boost::system::error_code err;
    socket_->connect(boost::asio::local::stream_protocol::endpoint(path), err);
    if (err) {
        const auto msg = utils::string_format("Failed to connect to socket %s: %s", path, err.message());
        BOOST_THROW_EXCEPTION(std::runtime_error(msg));
    }

    messenger_ = std::make_shared<SocketMessenger>(socket_);
}

LocalSocketMessenger::~LocalSocketMessenger() {
}

void LocalSocketMessenger::send(char const* data, size_t length) {
    messenger_->send(data, length);
}

void LocalSocketMessenger::async_receive_msg(AnboxReadHandler const& handle, boost::asio::mutable_buffers_1 const &buffer) {
    messenger_->async_receive_msg(handle, buffer);
}

boost::system::error_code LocalSocketMessenger::receive_msg(boost::asio::mutable_buffers_1 const& buffer) {
    return messenger_->receive_msg(buffer);
}

size_t LocalSocketMessenger::available_bytes() {
    return messenger_->available_bytes();
}
} // namespace network
} // namespace anbox
