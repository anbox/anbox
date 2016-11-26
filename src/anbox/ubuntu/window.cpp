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
#include "anbox/wm/window_state.h"
#include "anbox/logger.h"

#include <boost/throw_exception.hpp>

#include <SDL_syswm.h>

namespace {
constexpr const char* default_window_name{"anbox"};
}

namespace anbox {
namespace ubuntu {
Window::Id Window::Invalid{-1};

Window::Observer::~Observer() {
}

Window::Window(const Id &id,
               const wm::Task::Id &task,
               const std::shared_ptr<Observer> &observer,
               const graphics::Rect &frame) :
    wm::Window(task, frame),
    id_(id),
    observer_(observer),
    native_display_(0),
    native_window_(0) {

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);

    window_ = SDL_CreateWindow(default_window_name,
                               frame.left(),
                               frame.top(),
                               frame.width(),
                               frame.height(),
                               SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
    if (!window_) {
        const auto message = utils::string_format("Failed to create window: %s", SDL_GetError());
        BOOST_THROW_EXCEPTION(std::runtime_error(message));
    }

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window_, &info);
    switch (info.subsystem) {
    case SDL_SYSWM_X11:
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
}

Window::~Window() {
    if (window_)
        SDL_DestroyWindow(window_);

    if (observer_)
        observer_->window_deleted(id_);
}

void Window::resize(int width, int height) {
    SDL_SetWindowSize(window_, width, height);
}

void Window::update_position(int x, int y) {
    SDL_SetWindowPosition(window_, x, y);
}

void Window::process_event(const SDL_Event &event) {
    switch (event.window.event) {
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        if (observer_)
            observer_->window_wants_focus(id_);
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
        break;
    case SDL_WINDOWEVENT_RESIZED:
        break;
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        break;
    case SDL_WINDOWEVENT_MOVED:
        break;
    case SDL_WINDOWEVENT_SHOWN:
        break;
    case SDL_WINDOWEVENT_HIDDEN:
        break;
    }
}

EGLNativeWindowType Window::native_handle() const {
    return native_window_;
}

Window::Id Window::id() const {
    return id_;
}

std::uint32_t Window::window_id() const {
    return SDL_GetWindowID(window_);
}
} // namespace bridge
} // namespace anbox
