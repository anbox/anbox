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

#include "anbox/bridge/android_api_stub.h"
#include "anbox/system_configuration.h"
#include "anbox/logger.h"
#include "anbox/rpc/channel.h"
#include "anbox/utils.h"
#include "anbox/wm/stack.h"

#include "anbox_bridge.pb.h"
#include "anbox_rpc.pb.h"

#include <boost/filesystem.hpp>
#ifdef USE_PROTOBUF_CALLBACK_HEADER
#include <google/protobuf/stubs/callback.h>
#endif


namespace fs = boost::filesystem;

namespace {
constexpr const std::chrono::milliseconds default_rpc_call_timeout{30000};
} // namespace

namespace anbox {
namespace bridge {
AndroidApiStub::AndroidApiStub() {}

AndroidApiStub::~AndroidApiStub() {}

void AndroidApiStub::set_rpc_channel(
    const std::shared_ptr<rpc::Channel> &channel) {
  channel_ = channel;
}

void AndroidApiStub::reset_rpc_channel() { channel_.reset(); }

void AndroidApiStub::ensure_rpc_channel() {
  if (!channel_) throw std::runtime_error("No remote client connected");
}

void AndroidApiStub::launch(const android::Intent &intent,
                            const graphics::Rect &launch_bounds,
                            const wm::Stack::Id &stack) {
  ensure_rpc_channel();

  auto c = std::make_shared<Request<protobuf::rpc::Void>>();
  protobuf::bridge::LaunchApplication message;

  {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    launch_wait_handle_.expect_result();
  }

  switch (stack) {
  case wm::Stack::Id::Default:
    message.set_stack(::anbox::protobuf::bridge::LaunchApplication_Stack_DEFAULT);
    break;
  case wm::Stack::Id::Fullscreen:
    message.set_stack(::anbox::protobuf::bridge::LaunchApplication_Stack_FULLSCREEN);
    break;
  case wm::Stack::Id::Freeform:
    message.set_stack(::anbox::protobuf::bridge::LaunchApplication_Stack_FREEFORM);
    break;
  default:
    break;
  }

  if (launch_bounds != graphics::Rect::Invalid) {
    auto rect = message.mutable_launch_bounds();
    rect->set_left(launch_bounds_.left());
    rect->set_top(launch_bounds_.top());
    rect->set_right(launch_bounds_.right());
    rect->set_bottom(launch_bounds_.bottom());
  }

  auto launch_intent = message.mutable_intent();

  if (!intent.action.empty()) launch_intent->set_action(intent.action);

  if (!intent.uri.empty()) launch_intent->set_uri(intent.uri);

  if (!intent.type.empty()) launch_intent->set_type(intent.type);

  if (!intent.package.empty()) launch_intent->set_package(intent.package);

  if (!intent.component.empty()) launch_intent->set_component(intent.component);

  for (const auto &category : intent.categories) {
    auto c = launch_intent->add_categories();
    *c = category;
  }

  channel_->call_method(
      "launch_application", &message, c->response.get(),
      google::protobuf::NewCallback(this, &AndroidApiStub::application_launched,
                                    c.get()));

  launch_wait_handle_.wait_for_pending(default_rpc_call_timeout);
  if (!launch_wait_handle_.has_result())
    throw std::runtime_error("RPC call timed out");

  if (c->response->has_error()) throw std::runtime_error(c->response->error());
}

core::Property<bool>& AndroidApiStub::ready() {
  return ready_;
}

void AndroidApiStub::application_launched(
    Request<protobuf::rpc::Void> *request) {
  (void)request;
  launch_wait_handle_.result_received();
}

void AndroidApiStub::set_focused_task(const std::int32_t &id) {
  ensure_rpc_channel();

  auto c = std::make_shared<Request<protobuf::rpc::Void>>();

  protobuf::bridge::SetFocusedTask message;
  message.set_id(id);

  {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    set_focused_task_handle_.expect_result();
  }

  channel_->call_method("set_focused_task", &message, c->response.get(),
                        google::protobuf::NewCallback(
                            this, &AndroidApiStub::focused_task_set, c.get()));

  set_focused_task_handle_.wait_for_pending(default_rpc_call_timeout);
  if (!set_focused_task_handle_.has_result())
    throw std::runtime_error("RPC call timed out");

  if (c->response->has_error()) throw std::runtime_error(c->response->error());
}

void AndroidApiStub::focused_task_set(Request<protobuf::rpc::Void> *request) {
  (void)request;
  set_focused_task_handle_.result_received();
}

void AndroidApiStub::remove_task(const std::int32_t &id) {
  ensure_rpc_channel();

  auto c = std::make_shared<Request<protobuf::rpc::Void>>();

  protobuf::bridge::RemoveTask message;
  message.set_id(id);

  {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    remove_task_handle_.expect_result();
  }

  channel_->call_method("remove_task", &message, c->response.get(),
                        google::protobuf::NewCallback(
                            this, &AndroidApiStub::task_removed, c.get()));

  remove_task_handle_.wait_for_pending(default_rpc_call_timeout);
  if (!remove_task_handle_.has_result())
    throw std::runtime_error("RPC call timed out");

  if (c->response->has_error()) throw std::runtime_error(c->response->error());
}

void AndroidApiStub::task_removed(Request<protobuf::rpc::Void> *request) {
  (void)request;
  remove_task_handle_.result_received();
}

void AndroidApiStub::resize_task(const std::int32_t &id,
                                 const anbox::graphics::Rect &rect,
                                 const std::int32_t &resize_mode) {
  ensure_rpc_channel();

  auto c = std::make_shared<Request<protobuf::rpc::Void>>();

  protobuf::bridge::ResizeTask message;
  message.set_id(id);
  message.set_resize_mode(resize_mode);

  auto r = message.mutable_rect();
  r->set_left(rect.left());
  r->set_top(rect.top());
  r->set_right(rect.right());
  r->set_bottom(rect.bottom());

  {
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    resize_task_handle_.expect_result();
  }

  channel_->call_method("resize_task", &message, c->response.get(),
                        google::protobuf::NewCallback(
                            this, &AndroidApiStub::task_resized, c.get()));

  resize_task_handle_.wait_for_pending(default_rpc_call_timeout);
  if (!resize_task_handle_.has_result())
    throw std::runtime_error("RPC call timed out");

  if (c->response->has_error()) throw std::runtime_error(c->response->error());
}

void AndroidApiStub::task_resized(Request<protobuf::rpc::Void> *request) {
  (void)request;
  resize_task_handle_.result_received();
}
}  // namespace bridge
}  // namespace anbox
