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

#include "anbox/graphics/single_window_composer_strategy.h"
#include "anbox/wm/manager.h"
#include "anbox/utils.h"
#include "anbox/logger.h"

namespace {
const constexpr char *sprite_name{"Sprite"};
}

namespace anbox {
namespace graphics {
SingleWindowComposerStrategy::SingleWindowComposerStrategy(const std::shared_ptr<wm::Manager> &wm) : wm_(wm) {}

std::map<std::shared_ptr<wm::Window>, RenderableList> SingleWindowComposerStrategy::process_layers(const RenderableList &renderables) {
  WindowRenderableList win_layers;
  // FIXME there will be only one window in single-window mode ever so it
  // doesn't matter which task
  auto window = wm_->find_window_for_task(0);

  // Filter out any unwanted layers like the one responsible for the mouse
  // cursor which we don't want to render.
  RenderableList final_renderables;
  for (const auto &r : renderables) {
    if (r.name() == sprite_name)
      continue;
    final_renderables.push_back(r);
  }

  win_layers.insert({window, final_renderables});
  return win_layers;
}
}  // namespace graphics
}  // namespace anbox
