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

#include "anbox/wm/window_state.h"

namespace anbox {
namespace wm {
WindowState::WindowState()
    : display_(Display::Invalid),
      has_surface_(false),
      frame_(graphics::Rect::Invalid),
      package_name_(""),
      task_(Task::Invalid),
      stack_(Stack::Id::Invalid) {}

WindowState::WindowState(const Display::Id &display, bool has_surface,
                         const graphics::Rect &frame,
                         const std::string &package_name, const Task::Id &task,
                         const Stack::Id &stack)
    : display_(display),
      has_surface_(has_surface),
      frame_(frame),
      package_name_(package_name),
      task_(task),
      stack_(stack) {}

WindowState::~WindowState() {}
}  // namespace wm
}  // namespace anbox
