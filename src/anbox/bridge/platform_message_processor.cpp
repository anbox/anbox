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
#include "anbox/rpc/template_message_processor.h"

#include "anbox_bridge.pb.h"

namespace anbox {
namespace bridge {
PlatformMessageProcessor::PlatformMessageProcessor(const std::shared_ptr<network::MessageSender> &sender,
                                                   const std::shared_ptr<PlatformApiSkeleton> &server,
                                                   const std::shared_ptr<rpc::PendingCallCache> &pending_calls) :
    rpc::MessageProcessor(sender, pending_calls),
    server_(server) {
}

PlatformMessageProcessor::~PlatformMessageProcessor() {
}

void PlatformMessageProcessor::dispatch(rpc::Invocation const& invocation) {
    if (invocation.method_name() == "boot_finished")
        invoke(this, server_.get(), &PlatformApiSkeleton::boot_finished, invocation);
    else if (invocation.method_name() == "update_window_state")
        invoke(this, server_.get(), &PlatformApiSkeleton::update_window_state, invocation);
    else if (invocation.method_name() == "remove_window")
        invoke(this, server_.get(), &PlatformApiSkeleton::remove_window, invocation);
}

void PlatformMessageProcessor::process_event_sequence(const std::string&) {
}
} // namespace anbox
} // namespace network
