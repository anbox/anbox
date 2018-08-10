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

// Both includes need to go first as otherwise they can conflict with EGL.h
// being included by the following includes.
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "anbox/application/database.h"
#include "anbox/platform/base_platform.h"
#include "anbox/wm/multi_window_manager.h"
#include "anbox/wm/window_state.h"

#include "anbox/graphics/layer_composer.h"
#include "anbox/graphics/multi_window_composer_strategy.h"

using namespace ::testing;

namespace {
class MockRenderer : public anbox::graphics::Renderer {
 public:
  MOCK_METHOD3(draw, bool(EGLNativeWindowType, const anbox::graphics::Rect&,
                          const RenderableList&));
};
}

namespace anbox {
namespace graphics {
TEST(LayerComposer, FindsNoSuitableWindowForLayer) {
  auto renderer = std::make_shared<MockRenderer>();

  // The default policy will create a dumb window instance when requested
  // from the manager.
  auto platform = platform::create();
  auto app_db = std::make_shared<application::Database>();
  auto wm = std::make_shared<wm::MultiWindowManager>(platform, nullptr, app_db);

  auto single_window = wm::WindowState{
      wm::Display::Id{1},
      true,
      graphics::Rect{0, 0, 1024, 768},
      "org.anbox.test.1",
      wm::Task::Id{1},
      wm::Stack::Id::Freeform,
  };

  wm->apply_window_state_update({single_window}, {});

  LayerComposer composer(renderer, std::make_shared<MultiWindowComposerStrategy>(wm));

  // A single renderable which has a different task id then the window we know
  // about
  RenderableList renderables = {
      {"org.anbox.surface.2", 0, 1.0f, {0, 0, 1024, 768}, {0, 0, 1024, 768}},
  };

  // The renderer should not be called for a layer which doesn't exist
  EXPECT_CALL(*renderer, draw(_, _, _)).Times(0);

  composer.submit_layers(renderables);
}

TEST(LayerComposer, MapsLayersToWindows) {
  auto renderer = std::make_shared<MockRenderer>();

  // The default policy will create a dumb window instance when requested
  // from the manager.
  auto platform = platform::create();
  auto app_db = std::make_shared<application::Database>();
  auto wm = std::make_shared<wm::MultiWindowManager>(platform, nullptr, app_db);

  auto first_window = wm::WindowState{
      wm::Display::Id{1},
      true,
      graphics::Rect{0, 0, 1024, 768},
      "org.anbox.foo",
      wm::Task::Id{1},
      wm::Stack::Id::Freeform,
  };

  auto second_window = wm::WindowState{
      wm::Display::Id{1},
      true,
      graphics::Rect{300, 400, 1324, 1168},
      "org.anbox.bar",
      wm::Task::Id{2},
      wm::Stack::Id::Freeform,
  };

  wm->apply_window_state_update({first_window, second_window}, {});

  LayerComposer composer(renderer, std::make_shared<MultiWindowComposerStrategy>(wm));

  // A single renderable which has a different task id then the window we know
  // about
  RenderableList renderables = {
      {"org.anbox.surface.1", 0, 1.0f, {0, 0, 1024, 768}, {0, 0, 1024, 768}},
      {"org.anbox.surface.2", 1, 1.0f, {0, 0, 1024, 768}, {0, 0, 1024, 768}},
  };

  RenderableList first_window_renderables{
      {"org.anbox.surface.1", 0, 1.0f, {0, 0, 1024, 768}, {0, 0, 1024, 768}},
  };

  RenderableList second_window_renderables{
      {"org.anbox.surface.2", 1, 1.0f, {0, 0, 1024, 768}, {0, 0, 1024, 768}},
  };

  EXPECT_CALL(*renderer, draw(_, Rect{0, 0, first_window.frame().width(),
                                      first_window.frame().height()},
                              first_window_renderables))
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(*renderer, draw(_, Rect{0, 0, second_window.frame().width(),
                                      second_window.frame().height()},
                              second_window_renderables))
      .Times(1)
      .WillOnce(Return(true));

  composer.submit_layers(renderables);
}

TEST(LayerComposer, WindowPartiallyOffscreen) {
  auto renderer = std::make_shared<MockRenderer>();

  // The default policy will create a dumb window instance when requested
  // from the manager.
  auto platform = platform::create();
  auto app_db = std::make_shared<application::Database>();
  auto wm = std::make_shared<wm::MultiWindowManager>(platform, nullptr, app_db);

  auto window = wm::WindowState{
      wm::Display::Id{1},
      true,
      graphics::Rect{-100, -100, 924, 668},
      "org.anbox.foo",
      wm::Task::Id{1},
      wm::Stack::Id::Freeform,
  };

  wm->apply_window_state_update({window}, {});

  LayerComposer composer(renderer, std::make_shared<MultiWindowComposerStrategy>(wm));

  // Window is build out of two layers where one is placed inside the other
  // but the layer covering the whole window is placed with its top left
  // origin outside of the visible display area.
  RenderableList renderables = {
    {"org.anbox.surface.1", 0, 1.0f, {-100, -100, 924, 668}, {0, 0, 1024, 768}},
    {"org.anbox.surface.1", 1, 1.0f, {0, 0, 100, 200}, {0, 0, 100, 200}},
  };

  RenderableList expected_renderables{
    {"org.anbox.surface.1", 0, 1.0f, {0, 0, 1024, 768}, {0, 0, 1024, 768}},
    {"org.anbox.surface.1", 1, 1.0f, {100, 100, 200, 300}, {0, 0, 100, 200}},
  };

  EXPECT_CALL(*renderer, draw(_, Rect{0, 0,
                                      window.frame().width(),
                                      window.frame().height()},
                              expected_renderables))
      .Times(1)
      .WillOnce(Return(true));

  composer.submit_layers(renderables);
}

TEST(LayerComposer, PopupShouldNotCauseWindowLayerOffset) {
  auto renderer = std::make_shared<MockRenderer>();

  // The default policy will create a dumb window instance when requested
  // from the manager.
  auto platform = platform::create();
  auto app_db = std::make_shared<application::Database>();
  auto wm = std::make_shared<wm::MultiWindowManager>(platform, nullptr, app_db);

  auto window = wm::WindowState{
      wm::Display::Id{1},
      true,
      graphics::Rect{1120, 270, 2144, 1038},
      "org.anbox.foo",
      wm::Task::Id{3},
      wm::Stack::Id::Freeform,
  };

  wm->apply_window_state_update({window}, {});

  LayerComposer composer(renderer, std::make_shared<MultiWindowComposerStrategy>(wm));

  // Having two renderables where the second smaller one overlaps the bigger
  // one and goes a bit offscreen. This should be still placed correctly and
  // the small layer should go offscreen as it is supposed to. This typically
  // happens when a popup window appears which has a shadow attached which goes
  // out of the window area. In our case this is not possible as the area the
  // window has available is static.
  RenderableList renderables = {
    {"org.anbox.surface.3", 0, 1.0f, {1120,270,2144,1038}, {0, 0, 1024, 768}},
    {"org.anbox.surface.3", 1, 1.0f, {1904, 246, 2164, 406}, {0, 0, 260, 160}},
  };

  RenderableList expected_renderables{
    {"org.anbox.surface.3", 0, 1.0f, {0, 0, 1024, 768}, {0, 0, 1024, 768}},
    {"org.anbox.surface.3", 1, 1.0f, {784, -24, 1044, 136}, {0, 0, 260, 160}},
  };

  EXPECT_CALL(*renderer, draw(_, Rect{0, 0,
                                      window.frame().width(),
                                      window.frame().height()},
                              expected_renderables))
      .Times(1)
      .WillOnce(Return(true));

  composer.submit_layers(renderables);
}

}  // namespace graphics
}  // namespace anbox
