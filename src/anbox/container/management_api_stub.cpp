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

#include "anbox/container/management_api_stub.h"
#include "anbox/logger.h"
#include "anbox/rpc/channel.h"

#include "anbox_container.pb.h"
#include "anbox_rpc.pb.h"

#ifdef USE_PROTOBUF_CALLBACK_HEADER
#include <google/protobuf/stubs/callback.h>
#endif

namespace anbox {
namespace container {

const std::chrono::milliseconds ManagementApiStub::stop_waiting_timeout{3000};

ManagementApiStub::ManagementApiStub(
    const std::shared_ptr<rpc::Channel> &channel)
    : channel_(channel) {}

ManagementApiStub::~ManagementApiStub() {}

void ManagementApiStub::start_container(const Configuration &configuration) {
  auto c = std::make_shared<Request<protobuf::rpc::Void>>();

  protobuf::container::StartContainer message;
  auto message_configuration = new protobuf::container::Configuration;

  for (const auto &item : configuration.bind_mounts) {
    auto bind_mount_message = message_configuration->add_bind_mounts();
    bind_mount_message->set_source(item.first);
    bind_mount_message->set_target(item.second);
  }

  for (const auto &item : configuration.devices) {
    auto device_message = message_configuration->add_devices();
    device_message->set_path(item.first);
    device_message->set_permission(item.second.permission);
  }

  for (const auto &prop : configuration.extra_properties)
    message_configuration->add_extra_properties(prop);

  message.set_allocated_configuration(message_configuration);

  {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    c->wh.expect_result();
  }

  channel_->call_method("start_container", &message, c->response.get(),
      google::protobuf::NewCallback(this, &ManagementApiStub::container_started, c.get()));

  c->wh.wait_for_all();

  if (c->response->has_error()) throw std::runtime_error(c->response->error());
}

void ManagementApiStub::container_started(Request<protobuf::rpc::Void> *request) {
  request->wh.result_received();
}

void ManagementApiStub::stop_container() {
  auto c = std::make_shared<Request<protobuf::rpc::Void>>();

  protobuf::container::StopContainer message;
  message.set_force(false);

  {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    c->wh.expect_result();
  }

  channel_->call_method("stop_container", &message, c->response.get(),
      google::protobuf::NewCallback(this, &ManagementApiStub::container_stopped, c.get()));

  // If container manager dies before session manager, the session manager
  // cannot exit if it waits for all, so just wait for 3 seconds.
  c->wh.wait_for_pending(stop_waiting_timeout);

  if (c->response->has_error()) throw std::runtime_error(c->response->error());
}

void ManagementApiStub::container_stopped(Request<protobuf::rpc::Void> *request) {
  request->wh.result_received();
}

}  // namespace container
}  // namespace anbox
