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

#include "android/service/android_api_skeleton.h"

#include "anbox_rpc.pb.h"
#include "anbox_bridge.pb.h"

#include <core/posix/exec.h>
#include <core/posix/child_process.h>

#include <binder/IServiceManager.h>

namespace {
std::map<std::string,std::string> common_env = {
    {"ANDROID_DATA", "/data"},
    {"ANDROID_ROOT", "/system"},
};
}

namespace anbox {
AndroidApiSkeleton::AndroidApiSkeleton() {
}

AndroidApiSkeleton::~AndroidApiSkeleton() {
}

void AndroidApiSkeleton::wait_for_process(core::posix::ChildProcess &process,
                                   anbox::protobuf::rpc::Void *response) {
    const auto result = process.wait_for(core::posix::wait::Flags::untraced);
    if (result.status != core::posix::wait::Result::Status::exited ||
        result.detail.if_exited.status != core::posix::exit::Status::success) {
        response->set_error("Failed to install application");
        // FIXME once we add proper error codes/domains we need to add structured error
        // info the response here.
    }
}

void AndroidApiSkeleton::connect_services() {
    if (!activity_manager_.get()) {
        auto am = android::defaultServiceManager()->getService(android::String16("activity"));
        if (am.get())
            activity_manager_ = new android::BpActivityManager(am);
    }
}

void AndroidApiSkeleton::install_application(anbox::protobuf::bridge::InstallApplication const *request,
                                 anbox::protobuf::rpc::Void *response,
                                 google::protobuf::Closure *done) {
    (void) response;

    std::vector<std::string> argv = {
        "/system/bin/pm",
        "install",
        request->path(),
    };

    auto process = core::posix::exec("/system/bin/sh", argv, common_env, core::posix::StandardStream::empty);
    wait_for_process(process, response);

    done->Run();
}

void AndroidApiSkeleton::launch_application(anbox::protobuf::bridge::LaunchApplication const *request,
                                anbox::protobuf::rpc::Void *response,
                                google::protobuf::Closure *done) {
    (void) response;

    std::string intent = request->package_name();
    if (request->has_activity()) {
        intent += "/";
        intent += request->activity();
    }

    std::vector<std::string> argv = {
        "/system/bin/am",
        "start",
        // Launch any applications always in freeform stack
        "--stack", "2",
        intent,
    };

    auto process = core::posix::exec("/system/bin/sh", argv, common_env, core::posix::StandardStream::empty);
        wait_for_process(process, response);

    done->Run();
}

void AndroidApiSkeleton::set_dns_servers(anbox::protobuf::bridge::SetDnsServers const *request,
                             anbox::protobuf::rpc::Void *response,
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
    wait_for_process(process, response);

    done->Run();
}

void AndroidApiSkeleton::set_focused_task(anbox::protobuf::bridge::SetFocusedTask const *request,
                                          anbox::protobuf::rpc::Void *response,
                                          google::protobuf::Closure *done) {
    (void) response;

    connect_services();

    if (activity_manager_.get())
        activity_manager_->setFocusedTask(request->id());
    else
        response->set_error("ActivityManager is not available");

    done->Run();
}
} // namespace anbox
