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

#include "android/service/message_processor.h"
#include "android/service/server.h"

#include "anbox/bridge/template_message_processor.h"

namespace anbox {
namespace android {
MessageProcessor::MessageProcessor(const std::shared_ptr<network::MessageSender> &sender,
                                   const std::shared_ptr<bridge::PendingCallCache> &pending_calls,
                                   const std::shared_ptr<Server> &server) :
    bridge::MessageProcessor(sender, pending_calls),
    server_(server) {
}

MessageProcessor::~MessageProcessor() {
}

void MessageProcessor::dispatch(bridge::Invocation const& invocation) {
    if (invocation.method_name() == "install_application")
        invoke(this, server_.get(), &Server::install_application, invocation);
    else if (invocation.method_name() == "launch_application")
        invoke(this, server_.get(), &Server::launch_application, invocation);
}

void MessageProcessor::process_event_sequence(const std::string&) {
}
} // namespace anbox
} // namespace network
