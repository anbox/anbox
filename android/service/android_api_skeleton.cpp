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

#include <string>
#include <sstream>

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
        response->set_error("Failed to execute process");
        // FIXME once we add proper error codes/domains we need to add structured error
        // info to the response here.
    }
}

void AndroidApiSkeleton::connect_services() {
    if (!activity_manager_.get()) {
        auto am = android::defaultServiceManager()->getService(android::String16("activity"));
        if (am.get())
            activity_manager_ = new android::BpActivityManager(am);
    }
}

void AndroidApiSkeleton::launch_application(anbox::protobuf::bridge::LaunchApplication const *request,
                                anbox::protobuf::rpc::Void *response,
                                google::protobuf::Closure *done) {
    (void) response;

    auto intent = request->intent();

    std::vector<std::string> argv = {
        "/system/bin/am",
        "start",
    };

    if (request->has_stack()) {
      argv.push_back("--stack");
      argv.push_back(std::to_string(request->stack()));
    }

    if (request->has_launch_bounds()) {
        argv.push_back("--launch-bounds");
        std::stringstream launch_bounds;
        launch_bounds << request->launch_bounds().left() << " "
                      << request->launch_bounds().top() << " "
                      << request->launch_bounds().right() << " "
                      << request->launch_bounds().bottom();
        argv.push_back(launch_bounds.str());
    }

    if (intent.has_action()) {
        argv.push_back("-a");
        argv.push_back(intent.action());
    }

    if (intent.has_uri()) {
        argv.push_back("-d");
        argv.push_back(intent.uri());
    }

    if (intent.has_type()) {
        argv.push_back("-t");
        argv.push_back(intent.type());
    }

    std::string component;
    if (intent.has_package())
        component += intent.package();
    if (!component.empty() && intent.has_component()) {
        component += "/";
        component += intent.component();
    }

    if (!component.empty())
        argv.push_back(component);

    ALOGI("Launch am with the following arguments: ");
    std::string test;
    for (const auto &a : argv) {
        test += a;
        test += " ";
    }
    ALOGI("%s", test.c_str());

    auto process = core::posix::exec("/system/bin/sh", argv, common_env, core::posix::StandardStream::empty);
        wait_for_process(process, response);

    done->Run();
}

void AndroidApiSkeleton::set_focused_task(anbox::protobuf::bridge::SetFocusedTask const *request,
                                          anbox::protobuf::rpc::Void *response,
                                          google::protobuf::Closure *done) {
    connect_services();

    if (activity_manager_.get())
        activity_manager_->setFocusedTask(request->id());
    else
        response->set_error("ActivityManager is not available");

    done->Run();
}

void AndroidApiSkeleton::remove_task(anbox::protobuf::bridge::RemoveTask const *request,
                                     anbox::protobuf::rpc::Void *response,
                                     google::protobuf::Closure *done) {
  connect_services();

  if (activity_manager_.get())
    activity_manager_->removeTask(request->id());
  else
    response->set_error("ActivityManager is not available");

  done->Run();

}

void AndroidApiSkeleton::resize_task(anbox::protobuf::bridge::ResizeTask const *request,
                                     anbox::protobuf::rpc::Void *response,
                                     google::protobuf::Closure *done) {
  connect_services();

  if (activity_manager_.get()) {
    auto r = request->rect();
    activity_manager_->resizeTask(request->id(),
                                  anbox::graphics::Rect{r.left(), r.top(), r.right(), r.bottom()},
                                  request->resize_mode());
  } else {
    response->set_error("ActivityManager is not available");
  }

  done->Run();
}
} // namespace anbox
