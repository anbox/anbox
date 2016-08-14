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
#include "anbox/logger.h"

#include <boost/throw_exception.hpp>

namespace anbox {
namespace ubuntu {
WindowCreator::WindowCreator(const std::shared_ptr<input::Manager> &input_manager) :
    graphics::WindowCreator(input_manager),
    input_manager_(input_manager),
    event_thread_running_(false) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize SDL"));

    event_thread_ = std::thread(&WindowCreator::process_events, this);
}

WindowCreator::~WindowCreator() {
    event_thread_running_ = false;
    event_thread_.join();
}

void WindowCreator::process_window_event(const SDL_Event &event) {
}

void WindowCreator::process_events() {
    event_thread_running_ = true;

    while(event_thread_running_) {
        SDL_Event event;
        while (SDL_WaitEventTimeout(&event, 100)) {
            switch (event.type) {
            case SDL_QUIT:
                // FIXME: We exit here only as long as we don't have multi window
                // support.
                BOOST_THROW_EXCEPTION(std::runtime_error("User closed main application window"));
            case SDL_WINDOWEVENT:
                process_window_event(event);
                break;
            case SDL_FINGERUP:
            case SDL_FINGERMOTION:
            case SDL_FINGERDOWN:
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (current_window_)
                    current_window_->process_input_event(event);
                break;
            }
        }
    }
}

EGLNativeWindowType WindowCreator::create_window(int x, int y, int width, int height)
try {
    if (windows_.size() == 1) {
        WARNING("Tried to create another window but we currently only allow one");
        return 0;
    }

    auto window = std::make_shared<Window>(input_manager_, width, height);
    if (not window)
        BOOST_THROW_EXCEPTION(std::bad_alloc());

    windows_.insert({window->native_window(), window});
    current_window_ = window;

    return window->native_window();
}
catch (std::exception &err) {
    DEBUG("Failed to create window: %s", err.what());
    return 0;
}

void WindowCreator::destroy_window(EGLNativeWindowType win) {
    auto iter = windows_.find(win);
    if (iter == windows_.end())
        return;

    windows_.erase(iter);
}

WindowCreator::DisplayInfo WindowCreator::display_info() const {
    // FIXME: force for now until we have real detection for this
    return {1280, 720};
}

EGLNativeDisplayType WindowCreator::native_display() const {
    return  0;
}
} // namespace bridge
} // namespace anbox
