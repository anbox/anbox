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

#include "android/service/platform_api_stub.h"
#include "anbox/rpc/channel.h"

#include "anbox_rpc.pb.h"
#include "anbox_bridge.pb.h"

#define LOG_TAG "Anbox"
#include <cutils/log.h>

namespace anbox {
PlatformApiStub::PlatformApiStub(const std::shared_ptr<rpc::Channel> &rpc_channel) :
    rpc_channel_(rpc_channel) {
}

void PlatformApiStub::boot_finished() {
    auto c = std::make_shared<Request<protobuf::rpc::Void>>();

    ALOGI("Boot finished");

    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        boot_finished_wait_handle_.expect_result();
    }

    protobuf::rpc::Void message;

    rpc_channel_->call_method(
        "boot_finished",
        &message, c->response.get(),
        google::protobuf::NewCallback(this, &PlatformApiStub::handle_boot_finished_response, c.get()));

    boot_finished_wait_handle_.wait_for_all();

    ALOGI("Boot finished sent successfully!");
}

void PlatformApiStub::handle_boot_finished_response(Request<protobuf::rpc::Void>*) {
    boot_finished_wait_handle_.result_received();
}

void PlatformApiStub::update_window_state(const anbox::protobuf::bridge::WindowStateUpdate &window_state) {
    auto c = std::make_shared<Request<protobuf::rpc::Void>>();

    ALOGI("Updating window state");

    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        update_window_state_wait_handle_.expect_result();
    }

    rpc_channel_->call_method(
        "update_window_state",
        &window_state, c->response.get(),
        google::protobuf::NewCallback(this, &PlatformApiStub::handle_update_window_state_response, c.get()));

    update_window_state_wait_handle_.wait_for_all();
}

void PlatformApiStub::handle_update_window_state_response(Request<protobuf::rpc::Void> *request) {
    update_window_state_wait_handle_.result_received();
}
} // namespace anbox
