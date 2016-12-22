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

#include "anbox/wm/window.h"
#include "anbox/graphics/emugl/Renderer.h"
#include "anbox/logger.h"

namespace anbox {
namespace wm {
Window::Window(const std::shared_ptr<Renderer> &renderer, const Task::Id &task, const graphics::Rect &frame)
    : renderer_(renderer), task_(task), frame_(frame) {}

Window::~Window() {}

void Window::update_state(const WindowState::List &states) {
  (void)states;
}

void Window::update_frame(const graphics::Rect &frame) {
  if (frame == frame_) return;

  frame_ = frame;
}

Task::Id Window::task() const { return task_; }

graphics::Rect Window::frame() const { return frame_; }

EGLNativeWindowType Window::native_handle() const { return 0; }

bool Window::attach() {
  if (!renderer_)
    return false;
  return renderer_->createNativeWindow(native_handle());
}

void Window::release() {
  if (!renderer_)
    return;
  renderer_->destroyNativeWindow(native_handle());
}
}  // namespace wm
}  // namespace anbox
