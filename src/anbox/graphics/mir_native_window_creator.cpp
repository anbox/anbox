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

#include "anbox/graphics/mir_native_window_creator.h"
#include "anbox/graphics/mir_display_connection.h"
#include "anbox/graphics/mir_window.h"
#include "anbox/input/manager.h"

#include <boost/throw_exception.hpp>

namespace anbox {
namespace graphics {
MirNativeWindowCreator::MirNativeWindowCreator(const std::shared_ptr<input::Manager> &input_manager) :
    input_manager_(input_manager),
    display_connection_(std::make_shared<MirDisplayConnection>()) {
}

MirNativeWindowCreator::~MirNativeWindowCreator() {
}

EGLNativeWindowType MirNativeWindowCreator::create_window(int x, int y, int width, int height) {
    (void) x;
    (void) y;
    (void) width;
    (void) height;

    if (windows_.size() > 0)
        BOOST_THROW_EXCEPTION(std::runtime_error("We currently only allow a single native window"));

    auto window = std::make_shared<MirWindow>(display_connection_, input_manager_);
    windows_.insert({window->native_window(), window});

    return window->native_window();
}

void MirNativeWindowCreator::destroy_window(EGLNativeWindowType win) {
    auto iter = windows_.find(win);
    if (iter == windows_.end())
        return;

    windows_.erase(iter);
}

std::shared_ptr<MirDisplayConnection> MirNativeWindowCreator::display() const {
    return display_connection_;
}

} // namespace graphics
} // namespace anbox
