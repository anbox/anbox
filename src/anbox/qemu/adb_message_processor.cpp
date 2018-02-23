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

#include "anbox/qemu/adb_message_processor.h"
#include "anbox/network/delegate_connection_creator.h"
#include "anbox/network/delegate_message_processor.h"
#include "anbox/network/tcp_socket_messenger.h"
#include "anbox/utils.h"

#include <fstream>
#include <functional>

namespace {
const unsigned short default_adb_client_port{5037};
// For the listening port we have to use an odd port in the 5555-5585 range so
// the host can find us on start. See
// https://developer.android.com/studio/command-line/adb.html.
const unsigned short default_host_listen_port{5559};
constexpr const char *loopback_address{"127.0.0.1"};
const std::string accept_command{"accept"};
const std::string ok_command{"ok"};
const std::string ko_command{"ko"};
const std::string start_command{"start"};
// This timeount should be too high to not cause a too long wait time for the
// user until we connect to the adb host instance after it appeared and not
// too short to not put unnecessary burden on the CPU.
const boost::posix_time::seconds default_adb_wait_time{1};
}

using namespace std::placeholders;

namespace anbox {
namespace qemu {
std::mutex AdbMessageProcessor::active_instance_{};

AdbMessageProcessor::AdbMessageProcessor(
    const std::shared_ptr<Runtime> &rt,
    const std::shared_ptr<network::SocketMessenger> &messenger)
    : runtime_(rt),
      state_(waiting_for_guest_accept_command),
      expected_command_(accept_command),
      messenger_(messenger),
      lock_(active_instance_, std::defer_lock) {
}

AdbMessageProcessor::~AdbMessageProcessor() {
  state_ = closed_by_host;

  host_connector_.reset();
}

void AdbMessageProcessor::advance_state() {
  switch (state_) {
    case waiting_for_guest_accept_command:
      // Try to get a lock here as if we already have another processor
      // running we don't have to do anything here until that one is done.
      // The container directly starts a second connection once the first
      // one is established but will not use it until the active one is closed.
      lock_.lock();

      if (state_ == closed_by_host) {
        host_connector_.reset();
        return;
      }

      wait_for_host_connection();
      break;
    case waiting_for_host_connection:
      messenger_->send(reinterpret_cast<const char *>(ok_command.data()),
                       ok_command.size());
      state_ = waiting_for_guest_start_command;
      expected_command_ = start_command;
      break;
    case waiting_for_guest_start_command:
      state_ = proxying_data;
      read_next_host_message();
      break;
    case proxying_data:
      break;
    case closed_by_host:
      // Close the connection to the container as our adb host connection
      // turned down. The container will try to establish a connection
      // immediately again and we will handle that by waiting for the
      // host adb to run up again.
      messenger_->close();
      break;
    case closed_by_container:
      // In this case the container will close the pipe connection and this
      // message processor will be deleted once the owning socket connection
      // is closed.
      break;
    default:
      break;
  }
}

void AdbMessageProcessor::wait_for_host_connection() {
  if (state_ != waiting_for_guest_accept_command)
    return;

  if (!host_connector_) {
    host_connector_ = std::make_shared<network::TcpSocketConnector>(
        boost::asio::ip::address_v4::from_string(loopback_address),
        default_host_listen_port, runtime_,
        std::make_shared<
            network::DelegateConnectionCreator<boost::asio::ip::tcp>>(
            std::bind(&AdbMessageProcessor::on_host_connection, this, _1)));
  }

  try {
    // Notify the adb host instance so that it knows on which port our
    // proxy is waiting for incoming connections.
    auto messenger = std::make_shared<network::TcpSocketMessenger>(
        boost::asio::ip::address_v4::from_string(loopback_address), default_adb_client_port, runtime_);
    auto message = utils::string_format("host:emulator:%d", default_host_listen_port);
    auto handshake = utils::string_format("%04x%s", message.size(), message.c_str());
    messenger->send(handshake.data(), handshake.size());
  } catch (...) {
    // Server not up. No problem, it will contact us when started.
  }
}

void AdbMessageProcessor::on_host_connection(std::shared_ptr<boost::asio::basic_stream_socket<boost::asio::ip::tcp>> const &socket) {
  host_messenger_ = std::make_shared<network::TcpSocketMessenger>(socket);

  // set_no_delay() reduces the latency of sending data, at the cost
  // of creating more TCP packets on the connection. It's useful when
  // doing lots of small send() calls, like the ADB protocol requires.
  // And since this is on localhost, the packet increase should not be
  // noticeable.
  host_messenger_->set_no_delay();

  // Let adb inside the container know that we have a connection to
  // the adb host instance
  messenger_->send(reinterpret_cast<const char *>(ok_command.data()), ok_command.size());

  state_ = waiting_for_guest_start_command;
  expected_command_ = start_command;
}

void AdbMessageProcessor::read_next_host_message() {
  auto callback = std::bind(&AdbMessageProcessor::on_host_read_size, this, _1, _2);
  host_messenger_->async_receive_msg(callback, boost::asio::buffer(host_buffer_));
}

void AdbMessageProcessor::on_host_read_size(const boost::system::error_code &error, std::size_t bytes_read) {
  if (error) {
    // When AdbMessageProcessor is destroyed on program termination, the sockets
    // are closed and the standing operations are canceled. But, the callback is
    // still called even in that case, and the object has already been
    // deleted. We detect that condition by looking at the error code and avoid
    // touching *this in that case.
    if (error == boost::system::errc::operation_canceled)
      return;

    // For other errors, we assume the connection with the host is dropped. We
    // close the connection to the container's adbd, which will trigger the
    // deletion of this AdbMessageProcessor instance and free resources (most
    // importantly, default_host_listen_port and the lock). The standing
    // connection that adbd opened can then proceed and wait for the host to be
    // up again.
    state_ = closed_by_host;
    messenger_->close();
    return;
  }

  messenger_->send(reinterpret_cast<const char *>(host_buffer_.data()), bytes_read);
  read_next_host_message();
}

bool AdbMessageProcessor::process_data(const std::vector<std::uint8_t> &data) {
  if (state_ == proxying_data) {
    host_messenger_->send(reinterpret_cast<const char *>(data.data()),
                          data.size());
    return true;
  }

  for (const auto &byte : data) buffer_.push_back(byte);

  if (expected_command_.size() > 0 &&
      buffer_.size() >= expected_command_.size()) {
    if (::memcmp(buffer_.data(), expected_command_.data(), data.size()) != 0) {
      // We got not the command we expected and will terminate here
      return false;
    }

    buffer_.erase(buffer_.begin(), buffer_.begin() + expected_command_.size());
    expected_command_.clear();

    advance_state();
  }

  return true;
}
}  // namespace qemu
}  // namespace anbox
