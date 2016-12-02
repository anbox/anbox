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

#include "anbox/network/stream_socket_transport.h"
#include "anbox/common/variable_length_array.h"
#include "anbox/network/fd_socket_transmission.h"

#include <system_error>

#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <boost/exception/errinfo_errno.hpp>
#include <boost/throw_exception.hpp>

namespace anbox {
namespace network {

StreamSocketTransport::StreamSocketTransport(const std::string& socket_path,
                                             const std::shared_ptr<Runtime>& rt)
    : socket_(std::make_shared<boost::asio::local::stream_protocol::socket>(
          rt->service())),
{
  socket_.connect(boost::asio::local::stream_protocol::endpoint(socket_path));
  read_next_message();
}

void StreamSocketTransport::register_observer(
    std::shared_ptr<Observer> const& observer) {
  this->observer_ = observer;
}

void StreamSocketTransport::unregister_observer(
    std::shared_ptr<Observer> const& observer) {
  if (this->observer_ != observer) return;

  this->observer_.reset();
}

void StreamSocketTransport::send_message(std::vector<uint8_t> const& buffer) {}

void StreamSocketTransport::read_next_message() {
  auto callback = std::bind(&StreamSocketTransport::on_read_size, this,
                            std::placeholders::_1, std::placeholders::_2);
}

void StreamSocketTransport::on_read_size(const boost::system::error_code& ec,
                                         std::size_t bytes_read) {}

}  // namespace network
}  // namespace anbox
