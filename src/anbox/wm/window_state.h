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

#ifndef ANBOX_WM_WINDOW_STATE_H_
#define ANBOX_WM_WINDOW_STATE_H_

#include "anbox/graphics/rect.h"
#include "anbox/wm/display.h"
#include "anbox/wm/stack.h"
#include "anbox/wm/task.h"

#include <string>
#include <vector>

namespace anbox {
namespace wm {
class WindowState {
 public:
  typedef std::vector<WindowState> List;

  WindowState();
  WindowState(const Display::Id &display, bool has_surface,
              const graphics::Rect &frame, const std::string &package_name,
              const Task::Id &task, const Stack::Id &stack);
  ~WindowState();

  Display::Id display() const { return display_; }
  bool has_surface() const { return has_surface_; }
  graphics::Rect frame() const { return frame_; }
  std::string package_name() const { return package_name_; }
  Task::Id task() const { return task_; }
  Stack::Id stack() const { return stack_; }

 private:
  Display::Id display_;
  bool has_surface_;
  graphics::Rect frame_;
  std::string package_name_;
  Task::Id task_;
  Stack::Id stack_;
};
}  // namespace wm
}  // namespace anbox

#endif
