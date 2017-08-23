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

#ifndef ANBOX_QEMU_ADBD_MESSAGE_PROCESSOR_H_
#define ANBOX_QEMU_ADBD_MESSAGE_PROCESSOR_H_

#include "anbox/network/message_processor.h"
#include "anbox/network/socket_connection.h"
#include "anbox/network/socket_messenger.h"
#include "anbox/network/tcp_socket_connector.h"
#include "anbox/network/tcp_socket_messenger.h"
#include "anbox/runtime.h"

#include <boost/asio.hpp>

#include <mutex>

namespace anbox {
namespace qemu {
class AdbMessageProcessor : public network::MessageProcessor {
 public:
  AdbMessageProcessor(
      const std::shared_ptr<Runtime> &rt,
      const std::shared_ptr<network::SocketMessenger> &messenger);
  ~AdbMessageProcessor();

  bool process_data(const std::vector<std::uint8_t> &data) override;

 private:
  enum State {
    waiting_for_guest_accept_command,
    waiting_for_host_connection,
    waiting_for_guest_start_command,
    proxying_data,
    closed_by_container,
    closed_by_host,
  };

  void advance_state();

  void wait_for_host_connection();
  void on_host_connection(std::shared_ptr<boost::asio::basic_stream_socket<
                              boost::asio::ip::tcp>> const &socket);
  void read_next_host_message();
  void on_host_read_size(const boost::system::error_code &error,
                         std::size_t bytes_read);

  std::shared_ptr<Runtime> runtime_;
  State state_ = waiting_for_guest_accept_command;
  std::string expected_command_;
  std::shared_ptr<network::SocketMessenger> const messenger_;
  std::vector<std::uint8_t> buffer_;
  std::shared_ptr<network::TcpSocketConnector> host_connector_;
  std::shared_ptr<network::TcpSocketMessenger> host_messenger_;
  std::array<std::uint8_t, 8192> host_buffer_;
  std::unique_lock<std::mutex> lock_;

  static std::mutex active_instance_;
};
}  // namespace graphics
}  // namespace anbox

#endif
