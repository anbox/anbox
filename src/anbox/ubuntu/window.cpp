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

#include "anbox/ubuntu/window.h"
#include "anbox/logger.h"

#include <boost/throw_exception.hpp>

#include <SDL_syswm.h>

namespace anbox {
namespace ubuntu {
Window::Window(int x, int y, int width, int height) :
    native_display_(0),
    native_window_(0) {

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);

    window_ = SDL_CreateWindow("anbox", x, y, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
    if (!window_) {
        const auto message = utils::string_format("Failed to create window: %s", SDL_GetError());
        BOOST_THROW_EXCEPTION(std::runtime_error(message));
    }

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window_, &info);
    switch (info.subsystem) {
    case SDL_SYSWM_X11:
        DEBUG("Running on X11");
        native_display_ = static_cast<EGLNativeDisplayType>(info.info.x11.display);
        native_window_ = static_cast<EGLNativeWindowType>(info.info.x11.window);
        break;
    default:
        ERROR("Unknown subsystem (%d)", info.subsystem);
        BOOST_THROW_EXCEPTION(std::runtime_error("SDL subsystem not suported"));
    }

    int actual_width = 0, actual_height = 0;
    int actual_x = 0, actual_y = 0;
    SDL_GetWindowSize(window_, &actual_width, &actual_height);
    SDL_GetWindowPosition(window_, &actual_x, &actual_y);
    DEBUG("Window created {%d,%d,%d,%d}", actual_x, actual_y, actual_width, actual_height);
}

Window::~Window() {
    if (window_)
        SDL_DestroyWindow(window_);
}

EGLNativeWindowType Window::native_window() const {
    return native_window_;
}
} // namespace bridge
} // namespace anbox
