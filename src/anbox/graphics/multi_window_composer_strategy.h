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

#pragma once

#include "anbox/graphics/layer_composer.h"

#include <memory>

namespace anbox::graphics {
class MultiWindowComposerStrategy : public LayerComposer::Strategy {
 public:
  MultiWindowComposerStrategy(const std::shared_ptr<wm::Manager> &wm);
  ~MultiWindowComposerStrategy() = default;

  WindowRenderableList process_layers(const RenderableList &renderables) override;

private:
  std::shared_ptr<wm::Manager> wm_;
};
}
