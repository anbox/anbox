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

#ifndef ANBOX_BRIDGE_ANDROID_API_STUB_H_
#define ANBOX_BRIDGE_ANDROID_API_STUB_H_

#include "anbox/application/manager.h"
#include "anbox/common/wait_handle.h"
#include "anbox/graphics/rect.h"

#include <memory>
#include <vector>

namespace anbox {
namespace protobuf {
namespace rpc {
class Void;
}  // namespace bridge
}  // namespace protobuf
namespace rpc {
class Channel;
}  // namespace rpc
namespace bridge {
class AndroidApiStub : public anbox::application::Manager {
 public:
  AndroidApiStub();
  ~AndroidApiStub();

  void set_rpc_channel(const std::shared_ptr<rpc::Channel> &channel);
  void reset_rpc_channel();

  void set_focused_task(const std::int32_t &id);
  void remove_task(const std::int32_t &id);
  void resize_task(const std::int32_t &id, const anbox::graphics::Rect &rect,
                   const std::int32_t &resize_mode);

  void launch(const android::Intent &intent,
              const graphics::Rect &launch_bounds = graphics::Rect::Invalid,
              const wm::Stack::Id &stack = wm::Stack::Id::Default) override;

  core::Property<bool>& ready() override;

 private:
  void ensure_rpc_channel();

  template <typename Response>
  struct Request {
    Request() : response(std::make_shared<Response>()), success(true) {}
    std::shared_ptr<Response> response;
    bool success;
  };

  void application_launched(Request<protobuf::rpc::Void> *request);
  void focused_task_set(Request<protobuf::rpc::Void> *request);
  void task_removed(Request<protobuf::rpc::Void> *request);
  void task_resized(Request<protobuf::rpc::Void> *request);

  mutable std::mutex mutex_;
  std::shared_ptr<rpc::Channel> channel_;
  common::WaitHandle launch_wait_handle_;
  common::WaitHandle set_focused_task_handle_;
  common::WaitHandle remove_task_handle_;
  common::WaitHandle resize_task_handle_;
  graphics::Rect launch_bounds_ = graphics::Rect::Invalid;
  core::Property<bool> ready_;
};
}  // namespace bridge
}  // namespace anbox

#endif
