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

#include "anbox/wm/window.h"
#include "anbox/graphics/emugl/Renderer.h"

namespace anbox {
namespace wm {
Window::Window(const WindowState &state) :
    state_(state) {
}

Window::~Window() {
}

void Window::update_state(const WindowState &state) {
    state_ = state;
}

EGLNativeWindowType Window::native_handle() const {
    return 0;
}

bool Window::attach() {
    return Renderer::get()->createNativeWindow(native_handle());
}

void Window::release() {
    Renderer::get()->destroyNativeWindow(native_handle());
}
} // namespace wm
} // namespace anbox

