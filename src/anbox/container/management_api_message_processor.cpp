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

#include "anbox/container/management_api_message_processor.h"
#include "anbox/container/management_api_skeleton.h"
#include "anbox/rpc/template_message_processor.h"

#include "anbox_bridge.pb.h"
#include "anbox_container.pb.h"

namespace anbox {
namespace container {
ManagementApiMessageProcessor::ManagementApiMessageProcessor(
    const std::shared_ptr<network::MessageSender> &sender,
    const std::shared_ptr<rpc::PendingCallCache> &pending_calls,
    const std::shared_ptr<ManagementApiSkeleton> &server)
    : rpc::MessageProcessor(sender, pending_calls), server_(server) {}

ManagementApiMessageProcessor::~ManagementApiMessageProcessor() {}

void ManagementApiMessageProcessor::dispatch(rpc::Invocation const &invocation) {
  if (invocation.method_name() == "start_container")
    invoke(this, server_.get(), &ManagementApiSkeleton::start_container, invocation);
  else if (invocation.method_name() == "stop_container")
    invoke(this, server_.get(), &ManagementApiSkeleton::stop_container, invocation);
}

void ManagementApiMessageProcessor::process_event_sequence(
    const std::string &) {}
}  // namespace container
}  // namespace anbox
