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

#include <core/posix/exec.h>
#include <core/posix/child_process.h>

namespace {
std::map<std::string,std::string> common_env = {
    {"ANDROID_DATA", "/data"},
    {"ANDROID_ROOT", "/system"},
};
}

namespace anbox {
namespace android {
Server::Server() {
}

Server::~Server() {
}

void Server::install_application(anbox::protobuf::bridge::InstallApplication const *request,
                                 anbox::protobuf::bridge::Void *response,
                                 google::protobuf::Closure *done) {
    (void) response;

    std::vector<std::string> argv = {
        "/system/bin/pm",
        "install",
        request->path(),
    };

    auto process = core::posix::exec("/system/bin/sh", argv, common_env, core::posix::StandardStream::empty);
    process.wait_for(core::posix::wait::Flags::untraced);

    done->Run();
}

void Server::launch_application(anbox::protobuf::bridge::LaunchApplication const *request,
                                anbox::protobuf::bridge::Void *response,
                                google::protobuf::Closure *done) {
    (void) response;

    std::string intent = request->package_name();
    intent += "/";
    intent += request->activity();

    std::vector<std::string> argv = {
        "/system/bin/am",
        "start",
        intent,
    };

    auto process = core::posix::exec("/system/bin/sh", argv, common_env, core::posix::StandardStream::empty);
    process.wait_for(core::posix::wait::Flags::untraced);

    done->Run();
}

void Server::set_dns_servers(anbox::protobuf::bridge::SetDnsServers const *request,
                             anbox::protobuf::bridge::Void *response,
                             google::protobuf::Closure *done) {
    (void) response;

    std::vector<std::string> argv = {
        "resolver",
        "setnetdns",
        "0",
        request->domain(),
    };

    for (int n = 0; n < request->servers_size(); n++)
        argv.push_back(request->servers(n).address());

    auto process = core::posix::exec("/system/bin/ndc", argv, common_env, core::posix::StandardStream::empty);
    process.wait_for(core::posix::wait::Flags::untraced);

    done->Run();
}
} // namespace anbox
} // namespace network
