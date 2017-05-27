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

#include <gtest/gtest.h>

#include "anbox/graphics/emugl/DisplayManager.h"

extern int rcGetDisplayWidth(uint32_t display_id);
extern int rcGetDisplayHeight(uint32_t display_id);

TEST(RenderControl, WidthHeightAreCorrectlyAssigned) {
  anbox::graphics::emugl::DisplayInfo::get()->set_resolution(640, 480);
  ASSERT_EQ(rcGetDisplayWidth(0), 640);
  ASSERT_EQ(rcGetDisplayHeight(0), 480);
}
