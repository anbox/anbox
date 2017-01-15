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

#include "anbox/bridge/platform_message_processor.h"
#include "anbox/bridge/platform_api_skeleton.h"
#include "anbox/logger.h"
#include "anbox/rpc/template_message_processor.h"

#include "anbox_bridge.pb.h"

namespace anbox {
namespace bridge {
PlatformMessageProcessor::PlatformMessageProcessor(
    const std::shared_ptr<network::MessageSender> &sender,
    const std::shared_ptr<PlatformApiSkeleton> &server,
    const std::shared_ptr<rpc::PendingCallCache> &pending_calls)
    : rpc::MessageProcessor(sender, pending_calls), server_(server) {}

PlatformMessageProcessor::~PlatformMessageProcessor() {}

void PlatformMessageProcessor::dispatch(rpc::Invocation const &invocation) {
  if (invocation.method_name() == "set_clipboard_data")
    invoke(this, server_.get(), &PlatformApiSkeleton::set_clipboard_data, invocation);
  else if (invocation.method_name() == "get_clipboard_data")
    invoke(this, server_.get(), &PlatformApiSkeleton::get_clipboard_data, invocation);
}

void PlatformMessageProcessor::process_event_sequence(
    const std::string &raw_events) {
  anbox::protobuf::bridge::EventSequence seq;
  if (!seq.ParseFromString(raw_events)) {
    WARNING("Failed to parse events from raw string");
    return;
  }

  if (seq.has_boot_finished())
    server_->handle_boot_finished_event(seq.boot_finished());

  if (seq.has_window_state_update())
    server_->handle_window_state_update_event(seq.window_state_update());

  if (seq.has_application_list_update())
    server_->handle_application_list_update_event(
        seq.application_list_update());
}
}  // namespace anbox
}  // namespace network
