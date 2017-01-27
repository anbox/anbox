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

#ifndef ANBOX_WM_WINDOW_H_
#define ANBOX_WM_WINDOW_H_

#include "anbox/wm/window_state.h"

#include <string>
#include <vector>

#include <EGL/egl.h>

#include <memory>

class Renderer;

namespace anbox {
namespace wm {
// FIXME(morphis): move this somewhere else once we have the integration
// with the emugl layer.
class Layer {
 public:
  graphics::Rect frame() const { return frame_; }

 private:
  graphics::Rect frame_;
};

class Window {
 public:
  typedef std::vector<Window> List;

  Window(const std::shared_ptr<Renderer> &renderer, const Task::Id &task, const graphics::Rect &frame, const std::string &title);
  virtual ~Window();

  bool attach();
  void release();

  void update_state(const WindowState::List &states);
  void update_frame(const graphics::Rect &frame);

  virtual EGLNativeWindowType native_handle() const;
  graphics::Rect frame() const;
  Task::Id task() const;
  std::string title() const;

 private:
  std::shared_ptr<Renderer> renderer_;
  Task::Id task_;
  graphics::Rect frame_;
  std::string title_;
  bool attached_ = false;
};
}  // namespace wm
}  // namespace anbox

#endif
