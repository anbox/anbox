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

anbox::PlatformApiStub::WindowStateUpdate::Window PlatformService::unpack_window_state(const Parcel &data) {
    bool has_surface = data.readByte() != 0;

    String8 package_name(data.readString16());

    auto frame_left = data.readInt32();
    auto frame_top = data.readInt32();
    auto frame_right = data.readInt32();
    auto frame_bottom = data.readInt32();
    auto task_id = data.readInt32();
    auto stack_id = data.readInt32();

    return anbox::PlatformApiStub::WindowStateUpdate::Window{
        -1, // Display id will be added by the caller
        has_surface,
        package_name.string(),
        {frame_left, frame_top, frame_right, frame_bottom},
        task_id,
        stack_id,
    };
}

status_t PlatformService::update_window_state(const Parcel &data) {
    anbox::PlatformApiStub::WindowStateUpdate state;

    const auto num_displays = data.readInt32();
    for (auto n = 0; n < num_displays; n++) {
        const auto display_id = data.readInt32();
        const auto num_windows = data.readInt32();
        for (auto m = 0; m < num_windows; m++) {
            auto window = unpack_window_state(data);
            window.display_id = display_id;
            state.updated_windows.push_back(window);
        }
    }

    const auto num_removed_windows = data.readInt32();
    for (auto n = 0; n < num_removed_windows; n++) {
        auto window = unpack_window_state(data);
        state.removed_windows.push_back(window);
    }

    platform_api_stub_->update_window_state(state);

    return OK;
}

status_t PlatformService::update_application_list(const Parcel &data) {
    anbox::PlatformApiStub::ApplicationListUpdate update;
    const auto num_packages = data.readInt32();
    for (auto n = 0; n < num_packages; n++) {
        String8 name(data.readString16());
        String8 package_name(data.readString16());

        auto p = anbox::PlatformApiStub::ApplicationListUpdate::Application{
                name.string(),
                package_name.string(),
        };

        if (data.readInt32() == 1) {
            String8 action(data.readString16());
            String8 uri(data.readString16());
            String8 type(data.readString16());
            String8 component_package(data.readString16());
            String8 component_class(data.readString16());

            std::vector<std::string> categories;
            unsigned int num_categories = data.readInt32();
            for (int m = 0; m < num_categories; m++)
                categories.push_back(String8(data.readString16()).string());

            p.launch_intent.action = action;
            p.launch_intent.uri = uri;
            p.launch_intent.type = type;
            p.launch_intent.package = component_package;
            p.launch_intent.component = component_class;
            p.launch_intent.categories = categories;

            update.applications.push_back(p);
        };
    }

    platform_api_stub_->update_application_list(update);

    return OK;
}
} // namespace android
