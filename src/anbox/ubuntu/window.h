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

#ifndef ANBOX_UBUNTU_WINDOW_H_
#define ANBOX_UBUNTU_WINDOW_H_

#include <EGL/egl.h>

#include <memory>
#include <vector>

#include <SDL.h>

namespace anbox {
namespace input {
class Manager;
class Device;
class Event;
} // namespace input
namespace ubuntu {
class Window {
public:
    Window(const std::shared_ptr<input::Manager> &input_manager,
           int width, int height);
    ~Window();

    void process_input_event(const SDL_Event &event);

    EGLNativeWindowType native_window() const;

private:
    std::shared_ptr<input::Device> touchpanel_;
    std::shared_ptr<input::Device> pointer_;
    std::shared_ptr<input::Device> keyboard_;
    EGLNativeDisplayType native_display_;
    EGLNativeWindowType native_window_;
    SDL_Window *window_;
};
} // namespace bridge
} // namespace anbox

#endif
