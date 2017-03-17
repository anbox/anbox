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
Window::Window(const std::shared_ptr<Renderer> &renderer, const Task::Id &task, const graphics::Rect &frame, const std::string &title)
    : renderer_(renderer), task_(task), frame_(frame), title_(title) {}

Window::~Window() {
  release();
}

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

std::string Window::title() const { return title_; }

bool Window::attach() {
  if (!renderer_)
    return false;
  attached_ = renderer_->createNativeWindow(native_handle());
  return attached_;
}

void Window::release() {
  if (!renderer_ || !attached_)
    return;
  renderer_->destroyNativeWindow(native_handle());
}
}  // namespace wm
}  // namespace anbox
