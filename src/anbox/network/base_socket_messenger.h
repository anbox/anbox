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

#ifndef ANBOX_NETWORK_BASE_SOCKET_MESSENGER_H_
#define ANBOX_NETWORK_BASE_SOCKET_MESSENGER_H_

#include "anbox/common/fd_sets.h"
#include "anbox/network/socket_messenger.h"

#include <boost/asio.hpp>
#include <mutex>

namespace anbox {
namespace network {
template <typename stream_protocol>
class BaseSocketMessenger : public SocketMessenger {
 public:
  BaseSocketMessenger(
      std::shared_ptr<boost::asio::basic_stream_socket<stream_protocol>> const&
          socket);
  virtual ~BaseSocketMessenger();

  Credentials creds() const override;
  unsigned short local_port() const override;

  void send(char const* data, size_t length) override;
  ssize_t send_raw(char const* data, size_t length) override;
  void async_receive_msg(AnboxReadHandler const& handle,
                         boost::asio::mutable_buffers_1 const& buffer) override;
  boost::system::error_code receive_msg(
      boost::asio::mutable_buffers_1 const& buffer) override;
  size_t available_bytes() override;

  void set_no_delay() override;
  void close() override;

 protected:
  BaseSocketMessenger();
  void setup(std::shared_ptr<
             boost::asio::basic_stream_socket<stream_protocol>> const& s);

 private:
  std::shared_ptr<boost::asio::basic_stream_socket<stream_protocol>> socket;
  anbox::Fd socket_fd;
  std::mutex message_lock;
};
}  // namespace network
}  // namespace anbox

#endif
