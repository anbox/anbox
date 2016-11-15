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

#include "anbox/ubuntu/platform_api_skeleton.h"
#include "anbox/common/dispatcher.h"
#include "anbox/bridge/android_api_stub.h"
#include "anbox/rpc/pending_call_cache.h"
#include "anbox/logger.h"

#include "anbox_rpc.pb.h"
#include "anbox_bridge.pb.h"

namespace anbox {
namespace ubuntu {
PlatformApiSekeleton::PlatformApiSekeleton(const std::shared_ptr<rpc::PendingCallCache> &pending_calls) :
    bridge::PlatformApiSkeleton(pending_calls) {
}

PlatformApiSekeleton::~PlatformApiSekeleton() {
}

void PlatformApiSekeleton::boot_finished(anbox::protobuf::rpc::Void const *request,
                                         anbox::protobuf::rpc::Void *response,
                                         google::protobuf::Closure *done) {
    (void) request;
    (void) response;

    if (on_boot_finished_action_)
        on_boot_finished_action_();

    done->Run();
}

void PlatformApiSekeleton::update_window_state(anbox::protobuf::bridge::WindowStateUpdate const *request,
                                               anbox::protobuf::rpc::Void *response,
                                               google::protobuf::Closure *done) {
    (void) request;
    (void) response;

    DEBUG("");


    done->Run();
}

void PlatformApiSekeleton::on_boot_finished(const std::function<void()> &action) {
    on_boot_finished_action_ = action;
}
} // namespace ubuntu
} // namespace anbox
