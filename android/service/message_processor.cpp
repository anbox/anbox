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

#define LOG_TAG "Anboxd"

#include "android/service/message_processor.h"
#include "android/service/android_api_skeleton.h"

#include "anbox/rpc/template_message_processor.h"

#include "anbox_rpc.pb.h"
#include "anbox_bridge.pb.h"

namespace anbox {
MessageProcessor::MessageProcessor(const std::shared_ptr<network::MessageSender> &sender,
                                   const std::shared_ptr<rpc::PendingCallCache> &pending_calls,
                                   const std::shared_ptr<AndroidApiSkeleton> &platform_api) :
    rpc::MessageProcessor(sender, pending_calls),
    platform_api_(platform_api) {
}

MessageProcessor::~MessageProcessor() {
}

void MessageProcessor::dispatch(rpc::Invocation const& invocation) {
  if (invocation.method_name() == "launch_application")
    invoke(this, platform_api_.get(), &AndroidApiSkeleton::launch_application, invocation);
  else if (invocation.method_name() == "set_focused_task")
    invoke(this, platform_api_.get(), &AndroidApiSkeleton::set_focused_task, invocation);
  else if (invocation.method_name() == "remove_task")
    invoke(this, platform_api_.get(), &AndroidApiSkeleton::remove_task, invocation);
  else if (invocation.method_name() == "resize_task")
    invoke(this, platform_api_.get(), &AndroidApiSkeleton::resize_task, invocation);
}

void MessageProcessor::process_event_sequence(const std::string&) {
}
} // namespace network
