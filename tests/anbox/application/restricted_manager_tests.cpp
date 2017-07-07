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

#include "anbox/application/manager.h"

#include <gmock/gmock.h>

using namespace ::testing;

namespace {
class MockManager : public anbox::application::Manager {
 public:
  MOCK_METHOD3(launch, void(const anbox::android::Intent&,
                            const anbox::graphics::Rect&,
                            const anbox::wm::Stack::Id&));
  MOCK_METHOD0(ready, core::Property<bool>&());
};
}

TEST(RestrictedManager, RedirectsLaunchesToRightStack) {
  auto mgr = std::make_shared<MockManager>();
  anbox::application::RestrictedManager restricted_mgr(mgr, anbox::wm::Stack::Id::Freeform);

  EXPECT_CALL(*mgr, launch(_, _, anbox::wm::Stack::Id::Freeform))
      .Times(4);

  restricted_mgr.launch(anbox::android::Intent{},
                        anbox::graphics::Rect::Empty,
                        anbox::wm::Stack::Id::Default);

  restricted_mgr.launch(anbox::android::Intent{},
                        anbox::graphics::Rect::Empty,
                        anbox::wm::Stack::Id::Fullscreen);

  restricted_mgr.launch(anbox::android::Intent{},
                        anbox::graphics::Rect::Empty,
                        anbox::wm::Stack::Id::Invalid);

  restricted_mgr.launch(anbox::android::Intent{},
                        anbox::graphics::Rect::Empty,
                        anbox::wm::Stack::Id::Freeform);
}
