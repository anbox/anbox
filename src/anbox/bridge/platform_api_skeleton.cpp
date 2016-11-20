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

#include "anbox/bridge/platform_api_skeleton.h"
#include "anbox/wm/manager.h"
#include "anbox/wm/window_state.h"
#include "anbox/logger.h"

#include "anbox_bridge.pb.h"

namespace anbox {
namespace bridge {
PlatformApiSkeleton::PlatformApiSkeleton(const std::shared_ptr<rpc::PendingCallCache> &pending_calls,
                                         const std::shared_ptr<wm::Manager> &window_manager) :
    pending_calls_(pending_calls),
    window_manager_(window_manager) {
}

PlatformApiSkeleton::~PlatformApiSkeleton() {
}

void PlatformApiSkeleton::boot_finished(anbox::protobuf::rpc::Void const *request,
                                         anbox::protobuf::rpc::Void *response,
                                         google::protobuf::Closure *done) {
    (void) request;
    (void) response;

    if (on_boot_finished_action_)
        on_boot_finished_action_();

    done->Run();
}

void PlatformApiSkeleton::update_window_state(anbox::protobuf::bridge::WindowStateUpdate const *request,
                                               anbox::protobuf::rpc::Void *response,
                                               google::protobuf::Closure *done) {
    (void) response;

    auto convert_window_state = [](const ::anbox::protobuf::bridge::WindowStateUpdate_WindowState &window) {
        DEBUG("Window: display=%d has_surface=%d frame={%d,%d,%d,%d} package=%s task=%d stack=-1",
              window.display_id(), window.has_surface(),
              window.frame_left(), window.frame_top(), window.frame_right(), window.frame_bottom(),
              window.package_name(), window.task_id());
        return wm::WindowState(
                      wm::Display::Id(window.display_id()),
                      window.has_surface(),
                      graphics::Rect(window.frame_left(), window.frame_top(), window.frame_right(), window.frame_bottom()),
                      window.package_name(),
                      wm::Task::Id(window.task_id()),
                      wm::Stack::Id(wm::Stack::Invalid));
    };

    wm::WindowState::List updated;
    for (int n = 0; n < request->windows_size(); n++) {
        const auto window = request->windows(n);
        updated.push_back(convert_window_state(window));
    }

    wm::WindowState::List removed;
    for (int n = 0; n < request->removed_windows_size(); n++) {
        const auto window = request->removed_windows(n);
        removed.push_back(convert_window_state(window));
    }

    window_manager_->apply_window_state_update(updated, removed);

    done->Run();
}

void PlatformApiSkeleton::on_boot_finished(const std::function<void()> &action) {
    on_boot_finished_action_ = action;
}
} // namespace bridge
} // namespace anbox
