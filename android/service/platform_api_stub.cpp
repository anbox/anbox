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

namespace anbox {
PlatformApiStub::PlatformApiStub(const std::shared_ptr<rpc::Channel> &rpc_channel) :
    rpc_channel_(rpc_channel) {
}

void PlatformApiStub::boot_finished() {
    protobuf::bridge::EventSequence seq;
    auto event = seq.mutable_boot_finished();
    (void) event;
    rpc_channel_->send_event(seq);
}

void PlatformApiStub::update_window_state(const WindowStateUpdate &state) {
    protobuf::bridge::EventSequence seq;
    auto event = seq.mutable_window_state_update();

    auto convert_window = [](WindowStateUpdate::Window in, anbox::protobuf::bridge::WindowStateUpdateEvent_WindowState *out) {
        out->set_display_id(in.display_id);
        out->set_has_surface(in.has_surface);
        out->set_package_name(in.package_name);
        out->set_frame_left(in.frame.left);
        out->set_frame_top(in.frame.top);
        out->set_frame_right(in.frame.right);
        out->set_frame_bottom(in.frame.bottom);
        out->set_task_id(in.task_id);
        out->set_stack_id(in.stack_id);
    };

    for (const auto &window : state.updated_windows) {
        auto w = event->add_windows();
        convert_window(window, w);
    }

    for (const auto &window : state.removed_windows) {
        auto w = event->add_removed_windows();
        convert_window(window, w);
    }

    rpc_channel_->send_event(seq);
}

void PlatformApiStub::update_application_list(const ApplicationListUpdate &update) {
    protobuf::bridge::EventSequence seq;
    auto event = seq.mutable_application_list_update();

    for (const auto &a : update.applications) {
        auto app = event->add_applications();
        app->set_name(a.name);
        app->set_package(a.package);

        auto launch_intent = app->mutable_launch_intent();
        launch_intent->set_action(a.launch_intent.action);
        launch_intent->set_uri(a.launch_intent.uri);
        launch_intent->set_type(a.launch_intent.type);
        launch_intent->set_package(a.launch_intent.package);
        launch_intent->set_component(a.launch_intent.component);
        for (const auto &category : a.launch_intent.categories) {
            auto c = launch_intent->add_categories();
            *c = category;
        }

        app->set_icon(a.icon.data(), a.icon.size());
    }

    rpc_channel_->send_event(seq);
}
} // namespace anbox
