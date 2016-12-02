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

#include "anbox/wm/default_platform_policy.h"
#include "anbox/logger.h"
#include "anbox/wm/window.h"

namespace {
class NullWindow : public anbox::wm::Window {
 public:
  NullWindow(const anbox::wm::Task::Id &task,
             const anbox::graphics::Rect &frame)
      : anbox::wm::Window(0, frame) {}

 protected:
  void resize(int width, int height) override { WARNING("Not implemented"); }

  void update_position(int x, int y) override { WARNING("Not implemented"); }
};
}

namespace anbox {
namespace wm {
DefaultPlatformPolicy::DefaultPlatformPolicy() {}

std::shared_ptr<Window> DefaultPlatformPolicy::create_window(
    const anbox::wm::Task::Id &task, const anbox::graphics::Rect &frame) {
  DEBUG("");
  return std::make_shared<::NullWindow>(task, frame);
}
}  // namespace wm
}  // namespace anbox
