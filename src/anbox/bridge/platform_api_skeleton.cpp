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
#include "anbox/application/launcher_storage.h"
#include "anbox/platform/policy.h"
#include "anbox/wm/manager.h"
#include "anbox/wm/window_state.h"
#include "anbox/logger.h"

#include "anbox_bridge.pb.h"

namespace anbox {
namespace bridge {
PlatformApiSkeleton::PlatformApiSkeleton(
    const std::shared_ptr<rpc::PendingCallCache> &pending_calls,
    const std::shared_ptr<platform::Policy> &platform_policy,
    const std::shared_ptr<wm::Manager> &window_manager,
    const std::shared_ptr<application::LauncherStorage> &launcher_storage)
    : pending_calls_(pending_calls),
      platform_policy_(platform_policy),
      window_manager_(window_manager),
      launcher_storage_(launcher_storage) {}

PlatformApiSkeleton::~PlatformApiSkeleton() {}


void PlatformApiSkeleton::set_clipboard_data(anbox::protobuf::bridge::ClipboardData const *request,
                                             anbox::protobuf::rpc::Void *response,
                                             google::protobuf::Closure *done) {
  (void)response;

  if (request->has_text())
    platform_policy_->set_clipboard_data(platform::Policy::ClipboardData{request->text()});

  done->Run();
}

void PlatformApiSkeleton::get_clipboard_data(anbox::protobuf::rpc::Void const *request,
                                             anbox::protobuf::bridge::ClipboardData *response,
                                             google::protobuf::Closure *done) {
  (void)request;

  auto data = platform_policy_->get_clipboard_data();
  if (!data.text.empty())
    response->set_text(data.text);

  done->Run();
}

void PlatformApiSkeleton::handle_boot_finished_event(
    const anbox::protobuf::bridge::BootFinishedEvent &event) {
  (void)event;

  if (boot_finished_handler_) boot_finished_handler_();
}

void PlatformApiSkeleton::handle_window_state_update_event(
    const anbox::protobuf::bridge::WindowStateUpdateEvent &event) {
  auto convert_window_state = [](
      const ::anbox::protobuf::bridge::WindowStateUpdateEvent_WindowState
          &window) {
    return wm::WindowState(
        wm::Display::Id(window.display_id()), window.has_surface(),
        graphics::Rect(window.frame_left(), window.frame_top(),
                       window.frame_right(), window.frame_bottom()),
        window.package_name(), wm::Task::Id(window.task_id()),
        wm::Stack::Id(window.stack_id()));
  };

  wm::WindowState::List updated;
  for (int n = 0; n < event.windows_size(); n++) {
    const auto window = event.windows(n);
    updated.push_back(convert_window_state(window));
  }

  wm::WindowState::List removed;
  for (int n = 0; n < event.removed_windows_size(); n++) {
    const auto window = event.removed_windows(n);
    removed.push_back(convert_window_state(window));
  }

  window_manager_->apply_window_state_update(updated, removed);
}

void PlatformApiSkeleton::handle_application_list_update_event(
    const anbox::protobuf::bridge::ApplicationListUpdateEvent &event) {
  for (int n = 0; n < event.applications_size(); n++) {
    application::LauncherStorage::Item item;

    const auto app = event.applications(n);
    item.name = app.name();
    item.package = app.package();

    const auto li = app.launch_intent();
    item.launch_intent.action = li.action();
    item.launch_intent.uri = li.uri();
    item.launch_intent.type = li.uri();
    item.launch_intent.package = li.package();
    item.launch_intent.component = li.component();

    for (int m = 0; m < li.categories_size(); m++)
      item.launch_intent.categories.push_back(li.categories(m));

    item.icon = std::vector<char>(app.icon().begin(), app.icon().end());

    if (item.package.empty())
      continue;

    // If the item is already stored it will be updated
    launcher_storage_->add(item);
  }
}

void PlatformApiSkeleton::register_boot_finished_handler(
    const std::function<void()> &action) {
  boot_finished_handler_ = action;
}
}  // namespace bridge
}  // namespace anbox
