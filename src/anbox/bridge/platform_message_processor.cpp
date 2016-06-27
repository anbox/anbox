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
#include "anbox/bridge/platform_server.h"
#include "anbox/bridge/template_message_processor.h"

#include "anbox_bridge.pb.h"

namespace anbox {
namespace bridge {
PlatformMessageProcessor::PlatformMessageProcessor(const std::shared_ptr<network::MessageSender> &sender,
                                                   const std::shared_ptr<PlatformServer> &server,
                                                   const std::shared_ptr<PendingCallCache> &pending_calls) :
    MessageProcessor(sender, pending_calls),
    server_(server) {
}

PlatformMessageProcessor::~PlatformMessageProcessor() {
}

void PlatformMessageProcessor::dispatch(Invocation const& invocation) {
    if (invocation.method_name() == "handle_notification") {
        invoke(this, server_.get(), &PlatformServer::handle_notification, invocation);
    }
}

void PlatformMessageProcessor::process_event_sequence(const std::string&) {
}
} // namespace anbox
} // namespace network
