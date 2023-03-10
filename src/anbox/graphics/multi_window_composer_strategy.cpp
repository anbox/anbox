/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
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

#include "anbox/graphics/multi_window_composer_strategy.h"
#include "anbox/wm/manager.h"
#include "anbox/utils.h"

namespace anbox::graphics {
MultiWindowComposerStrategy::MultiWindowComposerStrategy(const std::shared_ptr<wm::Manager> &wm) : wm_(wm) {}

std::map<std::shared_ptr<wm::Window>, RenderableList> MultiWindowComposerStrategy::process_layers(const RenderableList &renderables) {
  WindowRenderableList win_layers;
  for (const auto &renderable : renderables) {
    // Ignore all surfaces which are not meant for a task
    if (!utils::string_starts_with(renderable.name(), "org.anbox.surface."))
      continue;

    wm::Task::Id task_id = 0;
    if (sscanf(renderable.name().c_str(), "org.anbox.surface.%d", &task_id) != 1 || !task_id)
      continue;

    auto w = wm_->find_window_for_task(task_id);
    if (!w) continue;

    if (win_layers.find(w) == win_layers.end()) {
      win_layers.insert({w, {renderable}});
      continue;
    }

    win_layers[w].push_back(renderable);
  }

  for (auto &w : win_layers) {
    const auto &renderables = w.second;
    RenderableList final_renderables;

    int32_t new_left   = INT_MAX;
    int32_t new_top    = INT_MAX;
    int32_t new_right  = INT_MIN;
    int32_t new_bottom = INT_MIN;

    for (auto &r : renderables) {
      // We always prioritize layers which are lower in the list we got
      // from SurfaceFlinger as they are already ordered.
      if (r.screen_position().left() < new_left)
        new_left = r.screen_position().left();
      if (r.screen_position().top() < new_top)
        new_top = r.screen_position().top();
      if (r.screen_position().right() > new_right) 
        new_right = r.screen_position().right();
      if (r.screen_position().bottom() > new_bottom)
        new_bottom = r.screen_position().bottom();
    }
    auto new_window_frame = Rect{new_left, new_top, new_right, new_bottom};

    for (auto &r : renderables) {
      // As we get absolute display coordinates from the Android hwcomposer we
      // need to recalculate all layer coordinates into relatives ones to the
      // window they are drawn into.
      auto rect = Rect{
          r.screen_position().left() - new_window_frame.left() + r.crop().left(),
          r.screen_position().top() - new_window_frame.top() + r.crop().top(),
          r.screen_position().right() - new_window_frame.left() + r.crop().left(),
          r.screen_position().bottom() - new_window_frame.top() + r.crop().top()};

      auto new_renderable = r;
      new_renderable.set_screen_position(rect);
      final_renderables.push_back(new_renderable);
    }

    w.second = final_renderables;
  }

  return win_layers;
}
}
