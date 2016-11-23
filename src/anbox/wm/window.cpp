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
#include "anbox/logger.h"

namespace anbox {
namespace wm {
Window::Window(const WindowState &state) :
    state_(state),
    refcount_(0) {
}

Window::~Window() {
}

void Window::update_state(const WindowState &new_state) {
    state_ = new_state;
}

WindowState Window::state() const {
    return state_;
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

void Window::ref() {
    refcount_++;
}

void Window::unref() {
    if (refcount_ == 0) {
        WARNING("reference count is out of sync");
        return;
    }

    refcount_--;
}

bool Window::still_used() const {
    return refcount_ > 0;
}
} // namespace wm
} // namespace anbox

