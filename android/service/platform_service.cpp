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

#include "android/service/platform_service.h"
#include "android/service/platform_api_stub.h"
#include "anbox/rpc/channel.h"

#include "anbox_rpc.pb.h"
#include "anbox_bridge.pb.h"

#define LOG_TAG "Anboxd"
#include <cutils/log.h>

#include <utils/String8.h>

using namespace android;

namespace android {
PlatformService::PlatformService(const std::shared_ptr<anbox::PlatformApiStub> &platform_api_stub) :
    platform_api_stub_(platform_api_stub) {
}

status_t PlatformService::boot_finished() {
    platform_api_stub_->boot_finished();
    return OK;
}

void PlatformService::unpack_window_state(anbox::protobuf::bridge::WindowStateUpdate_WindowState *window, const Parcel &data) {
    window->set_has_surface(data.readByte() != 0);

    String8 package_name(data.readString16());

    auto frame_left = data.readInt32();
    auto frame_top = data.readInt32();
    auto frame_right = data.readInt32();
    auto frame_bottom = data.readInt32();
    auto task_id = data.readInt32();
    auto stack_id = data.readInt32();

    window->set_package_name(package_name.string());
    window->set_frame_left(frame_left);
    window->set_frame_top(frame_top);
    window->set_frame_right(frame_right);
    window->set_frame_bottom(frame_bottom);
    window->set_task_id(task_id);
    window->set_stack_id(stack_id);
}

status_t PlatformService::update_window_state(const Parcel &data) {
    anbox::protobuf::bridge::WindowStateUpdate window_state;

    const auto num_displays = data.readInt32();
    for (auto n = 0; n < num_displays; n++) {
        const auto display_id = data.readInt32();
        const auto num_windows = data.readInt32();

        for (auto m = 0; m < num_windows; m++) {
            auto window = window_state.add_windows();
            window->set_display_id(display_id);
            unpack_window_state(window, data);
        }
    }

    const auto num_removed_windows = data.readInt32();
    for (auto n = 0; n < num_removed_windows; n++) {
        auto window = window_state.add_removed_windows();
        window->set_display_id(0);
        unpack_window_state(window, data);
    }

    platform_api_stub_->update_window_state(window_state);

    return OK;
}
} // namespace android
