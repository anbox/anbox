/*
 * Copyright © 2013-2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "anbox/network/socket_messenger.h"
#include "anbox/common/variable_length_array.h"
#include "anbox/logger.h"

#include <boost/throw_exception.hpp>

#include <errno.h>
#include <string.h>

#include <stdexcept>

namespace bs = boost::system;
namespace ba = boost::asio;

namespace {
/// Buffers need to be big enough to support messages
unsigned int const serialization_buffer_size = 2048;
}

namespace anbox {
namespace network {
SocketMessenger::SocketMessenger(std::shared_ptr<ba::local::stream_protocol::socket> const& socket)
    : socket(socket),
      socket_fd{IntOwnedFd{socket->native_handle()}}
{
    socket->non_blocking(true);
    boost::asio::socket_base::send_buffer_size option(64*1024);
    socket->set_option(option);
}

void SocketMessenger::send(char const* data, size_t length)
{
    VariableLengthArray<serialization_buffer_size> whole_message{length};
    std::copy(data, data + length, whole_message.data());

    try {
        std::unique_lock<std::mutex> lg(message_lock);
        ba::write(*socket,
                  ba::buffer(whole_message.data(), whole_message.size()),
                  boost::asio::transfer_all());
    }
    catch (std::exception &err) {
        ERROR("Failed to write message: %s", err.what());
    }
}

void SocketMessenger::async_receive_msg(
    AnboxReadHandler const& handler,
    ba::mutable_buffers_1 const& buffer)
{
    socket->async_read_some(buffer, handler);
}

bs::error_code SocketMessenger::receive_msg(
    ba::mutable_buffers_1 const& buffer)
{
    bs::error_code e;
    size_t nread = 0;

    while (nread < ba::buffer_size(buffer))
    {
        nread += boost::asio::read(
             *socket,
             ba::mutable_buffers_1{buffer + nread},
             e);

        if (e && e != ba::error::would_block)
            break;
    }

    return e;
}

size_t SocketMessenger::available_bytes()
{
    boost::asio::socket_base::bytes_readable command{true};
    socket->io_control(command);
    return command.get();
}
} // namespace network
} // namespace anbox
