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

#ifndef ANBOX_BRIDGE_PLATFORM_SERVER_H_
#define ANBOX_BRIDGE_PLATFORM_SERVER_H_

#include <functional>
#include <memory>

namespace google {
namespace protobuf {
class Closure;
}  // namespace protobuf
}  // namespace google

namespace anbox {
namespace protobuf {
namespace rpc {
class Void;
}  // namespace rpc
namespace bridge {
class ClipboardData;
class BootFinishedEvent;
class WindowStateUpdateEvent;
class ApplicationListUpdateEvent;
}  // namespace bridge
}  // namespace protobuf
namespace platform {
class BasePlatform;
} // namespace platform
namespace rpc {
class PendingCallCache;
}  // namespace rpc
namespace wm {
class Manager;
}  // namespace wm
namespace application {
class Database;
}  // namespace application
namespace bridge {
class PlatformApiSkeleton {
 public:
  PlatformApiSkeleton(
      const std::shared_ptr<rpc::PendingCallCache> &pending_calls,
      const std::shared_ptr<platform::BasePlatform> &platform,
      const std::shared_ptr<wm::Manager> &window_manager,
      const std::shared_ptr<application::Database> &app_db);
  virtual ~PlatformApiSkeleton();

  void set_clipboard_data(anbox::protobuf::bridge::ClipboardData const *request,
                          anbox::protobuf::rpc::Void *response,
                          google::protobuf::Closure *done);
  void get_clipboard_data(anbox::protobuf::rpc::Void const *request,
                          anbox::protobuf::bridge::ClipboardData *response,
                          google::protobuf::Closure *done);

  void handle_boot_finished_event(
      const anbox::protobuf::bridge::BootFinishedEvent &event);
  void handle_window_state_update_event(
      const anbox::protobuf::bridge::WindowStateUpdateEvent &event);
  void handle_application_list_update_event(
      const anbox::protobuf::bridge::ApplicationListUpdateEvent &event);

  void register_boot_finished_handler(const std::function<void()> &action);

 private:
  std::shared_ptr<rpc::PendingCallCache> pending_calls_;
  std::shared_ptr<platform::BasePlatform> platform_;
  std::shared_ptr<wm::Manager> window_manager_;
  std::shared_ptr<application::Database> app_db_;
  std::function<void()> boot_finished_handler_;
};
}  // namespace bridge
}  // namespace anbox

#endif
