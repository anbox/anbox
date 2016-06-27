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

#include "android/service/server.h"

#include "anbox_bridge.pb.h"

namespace anbox {
namespace android {
Server::Server() {
}

Server::~Server() {
}

void Server::install_application(anbox::protobuf::bridge::InstallApplication const *request,
                                 anbox::protobuf::bridge::Void *response,
                                 google::protobuf::Closure *done) {
    (void) request;
    (void) response;

    done->Run();
}

void Server::launch_application(anbox::protobuf::bridge::LaunchApplication const *request,
                                anbox::protobuf::bridge::Void *response,
                                google::protobuf::Closure *done) {
    (void) request;
    (void) response;

    done->Run();
}

} // namespace anbox
} // namespace network
