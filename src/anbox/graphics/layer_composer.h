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

#ifndef ANBOX_GRAPHICS_LAYER_COMPOSER_H_
#define ANBOX_GRAPHICS_LAYER_COMPOSER_H_

#include "anbox/graphics/renderer.h"

#include <memory>
#include <map>

namespace anbox {
namespace wm {
class Manager;
class Window;
}  // namespace wm
namespace graphics {
class LayerComposer {
 public:
  class Strategy {
   public:
    typedef std::map<std::shared_ptr<wm::Window>, RenderableList> WindowRenderableList;

    virtual ~Strategy() {}
    virtual WindowRenderableList process_layers(const RenderableList &renderables) = 0;
  };

  LayerComposer(const std::shared_ptr<Renderer> renderer,
                const std::shared_ptr<Strategy> &strategy);
  ~LayerComposer();

  void submit_layers(const RenderableList &renderables);

 private:
  std::shared_ptr<Renderer> renderer_;
  std::shared_ptr<Strategy> strategy_;
};
}  // namespace graphics
}  // namespace anbox

#endif
