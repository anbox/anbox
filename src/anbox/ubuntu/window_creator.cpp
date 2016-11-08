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
#include "anbox/input/manager.h"
#include "anbox/input/device.h"
#include "anbox/logger.h"

#include <boost/throw_exception.hpp>

#include <sys/types.h>
#include <signal.h>

namespace anbox {
namespace ubuntu {
WindowCreator::WindowCreator(const std::shared_ptr<input::Manager> &input_manager) :
    graphics::WindowCreator(input_manager),
    input_manager_(input_manager),
    event_thread_running_(false),
    display_info_({1920, 1080}) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize SDL"));

    event_thread_ = std::thread(&WindowCreator::process_events, this);

    SDL_DisplayMode display_mode;
    // FIXME statically just check the first (primary) display for its mode;
    // once we get multi-monitor support we need to do this better.
    if (SDL_GetCurrentDisplayMode(0, &display_mode) == 0) {
        display_info_.horizontal_resolution = display_mode.w;
        display_info_.vertical_resolution = display_mode.h;
    }
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
                // This is the best way to reliable terminate the whole application for now. It will
                // trigger a correct shutdown in the main part.
                ::kill(getpid(), SIGTERM);
                break;
            case SDL_WINDOWEVENT:
                process_window_event(event);
                break;
            }
        }
    }
}

EGLNativeWindowType WindowCreator::create_window(int x, int y, int width, int height)
try {
    auto window = std::make_shared<Window>(x, y, width, height);
    if (not window)
        BOOST_THROW_EXCEPTION(std::bad_alloc());

    windows_.insert({window->native_window(), window});

    return window->native_window();
}
catch (std::exception &err) {
    DEBUG("Failed to create window: %s", err.what());
    return 0;
}

void WindowCreator::update_window(EGLNativeWindowType win, int x, int y, int width, int height) {
    auto iter = windows_.find(win);
    if (iter == windows_.end())
        return;

    iter->second->resize(width, height);
    iter->second->update_position(x, y);
}

void WindowCreator::destroy_window(EGLNativeWindowType win) {
    auto iter = windows_.find(win);
    if (iter == windows_.end())
        return;

    windows_.erase(iter);
}

WindowCreator::DisplayInfo WindowCreator::display_info() const {
    return display_info_;
}

EGLNativeDisplayType WindowCreator::native_display() const {
    return  0;
}
} // namespace bridge
} // namespace anbox
