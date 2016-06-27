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

#include "anbox/ubuntu/window_creator.h"
#include "anbox/ubuntu/window.h"
#include "anbox/ubuntu/mir_display_connection.h"
#include "anbox/logger.h"

#include <boost/throw_exception.hpp>

namespace anbox {
namespace ubuntu {
WindowCreator::WindowCreator(const std::shared_ptr<input::Manager> &input_manager) :
    graphics::WindowCreator(input_manager),
    input_manager_(input_manager),
    display_(std::make_shared<MirDisplayConnection>()) {
}

WindowCreator::~WindowCreator() {
}

EGLNativeWindowType WindowCreator::create_window(int x, int y, int width, int height) {
    DEBUG("x %i y %i width %i height %i", x, y, width, height);

    auto window = std::make_shared<Window>(display_, input_manager_,
                                           width, height);
    windows_.insert({window->native_window(), window});

    return window->native_window();
}

void WindowCreator::destroy_window(EGLNativeWindowType win) {
    auto iter = windows_.find(win);
    if (iter == windows_.end())
        return;

    windows_.erase(iter);
}

WindowCreator::DisplayInfo WindowCreator::display_info() const {
    return {display_->horizontal_resolution(), display_->vertical_resolution()};
}
} // namespace bridge
} // namespace anbox
