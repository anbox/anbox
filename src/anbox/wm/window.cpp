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
Window::Window(const Task::Id &task, const graphics::Rect &frame)
    : task_(task), frame_(frame) {}

Window::~Window() {}

void Window::update_state(const WindowState::List &states) {}

void Window::update_frame(const graphics::Rect &frame) {
  if (frame == frame_) return;

  if (frame.width() != frame_.width() || frame.height() != frame_.height())
    resize(frame.width(), frame.height());

  if (frame.top() != frame_.top() || frame.left() != frame_.left())
    update_position(frame.left(), frame.top());

  frame_ = frame;
}

Task::Id Window::task() const { return task_; }

graphics::Rect Window::frame() const { return frame_; }

EGLNativeWindowType Window::native_handle() const { return 0; }

bool Window::attach() {
  return Renderer::get()->createNativeWindow(native_handle());
}

void Window::release() {
  Renderer::get()->destroyNativeWindow(native_handle());
}
}  // namespace wm
}  // namespace anbox
