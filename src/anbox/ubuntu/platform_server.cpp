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

#include "anbox/ubuntu/platform_server.h"
#include "anbox/logger.h"

#include "bridge.pb.h"

namespace anbox {
namespace ubuntu {
PlatformServer::PlatformServer(const std::shared_ptr<bridge::PendingCallCache> &pending_calls) :
    bridge::PlatformServer(pending_calls) {
}

PlatformServer::~PlatformServer() {
}

void PlatformServer::handle_notification(anbox::protobuf::bridge::Notification const *request,
                                         anbox::protobuf::bridge::Void *response,
                                         google::protobuf::Closure *done) {
    (void) request;
    (void) response;
    DEBUG("");
    done->Run();
}

} // namespace ubuntu
} // namespace anbox
