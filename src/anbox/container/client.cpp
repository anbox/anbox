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

#include "anbox/container/client.h"
#include "anbox/system_configuration.h"
#include "anbox/container/management_api_stub.h"
#include "anbox/logger.h"
#include "anbox/network/local_socket_messenger.h"
#include "anbox/rpc/channel.h"
#include "anbox/rpc/message_processor.h"
#include "anbox/rpc/pending_call_cache.h"

namespace ba = boost::asio;
namespace bs = boost::system;

namespace anbox {
namespace container {
Client::Client(const std::shared_ptr<Runtime> &rt)
    : messenger_(std::make_shared<network::LocalSocketMessenger>(
          SystemConfiguration::instance().container_socket_path(), rt)),
      pending_calls_(std::make_shared<rpc::PendingCallCache>()),
      rpc_channel_(std::make_shared<rpc::Channel>(pending_calls_, messenger_)),
      management_api_(std::make_shared<ManagementApiStub>(rpc_channel_)),
      processor_(
          std::make_shared<rpc::MessageProcessor>(messenger_, pending_calls_)) {
  read_next_message();
}

Client::~Client() {}

void Client::start(const Configuration &configuration) {
  try {
    management_api_->start_container(configuration);
  } catch (const std::exception &e) {
    ERROR("Failed to start container: %s", e.what());
    if (terminate_callback_)
      terminate_callback_();
  }
}

void Client::stop() {
  management_api_->stop_container();
}

void Client::register_terminate_handler(const TerminateCallback &callback) {
  terminate_callback_ = callback;
}

void Client::read_next_message() {
  auto callback = std::bind(&Client::on_read_size, this, std::placeholders::_1,
                            std::placeholders::_2);
  messenger_->async_receive_msg(callback, ba::buffer(buffer_));
}

void Client::on_read_size(const boost::system::error_code &error,
                          std::size_t bytes_read) {
  if (error) {
    if (terminate_callback_)
      terminate_callback_();
    return;
  }

  std::vector<std::uint8_t> data(bytes_read);
  std::copy(buffer_.data(), buffer_.data() + bytes_read, data.data());

  if (processor_->process_data(data)) read_next_message();
}
}  // namespace container
}  // namespace anbox
