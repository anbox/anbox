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

#include "anbox/container/management_api_skeleton.h"
#include "anbox/container/configuration.h"
#include "anbox/container/container.h"
#include "anbox/defer_action.h"
#include "anbox/logger.h"
#include "anbox/utils.h"

#include "anbox_container.pb.h"
#include "anbox_rpc.pb.h"

namespace anbox {
namespace container {
ManagementApiSkeleton::ManagementApiSkeleton(
    const std::shared_ptr<rpc::PendingCallCache> &pending_calls,
    const std::shared_ptr<Container> &container)
    : pending_calls_(pending_calls), container_(container) {}

ManagementApiSkeleton::~ManagementApiSkeleton() {}

void ManagementApiSkeleton::start_container(
    anbox::protobuf::container::StartContainer const *request,
    anbox::protobuf::rpc::Void *response, google::protobuf::Closure *done) {
  DEBUG("");

  if (container_->state() == Container::State::running) {
    response->set_error("Container is already running");
    done->Run();
    return;
  }

  Configuration container_configuration;

  const auto configuration = request->configuration();
  for (int n = 0; n < configuration.bind_mounts_size(); n++) {
    const auto bind_mount = configuration.bind_mounts(n);
    container_configuration.bind_mounts.insert({bind_mount.source(), bind_mount.target()});
  }

  for (int n = 0; n < configuration.devices_size(); n++) {
    const auto device = configuration.devices(n);
    container_configuration.devices.insert({device.path(), {device.permission()}});
  }

  for (int n = 0; n < configuration.extra_properties_size(); n++) {
    const auto prop = configuration.extra_properties(n);
    container_configuration.extra_properties.push_back(prop);
  }

  try {
    container_->start(container_configuration);
  } catch (std::exception &err) {
    response->set_error(utils::string_format("Failed to start container: %s", err.what()));
  }

  done->Run();
}

void ManagementApiSkeleton::stop_container(
    anbox::protobuf::container::StopContainer const *request,
    anbox::protobuf::rpc::Void *response, google::protobuf::Closure *done) {

  (void)request;

  if (container_->state() != Container::State::running) {
    response->set_error("Container is not running");
    done->Run();
    return;
  }

  try {
    container_->stop();
  } catch (std::exception &err) {
    response->set_error(utils::string_format("Failed to stop container: %s", err.what()));
  }

  done->Run();
}
}  // namespace container
}  // namespace anbox
