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

#ifndef ANBOX_UBUNTU_WINDOW_CREATOR_H_
#define ANBOX_UBUNTU_WINDOW_CREATOR_H_

#include "anbox/graphics/window_creator.h"

#include <map>
#include <thread>

#include <EGL/egl.h>
#include <SDL.h>

namespace anbox {
namespace ubuntu {
class MirDisplayConnection;
class Window;
class WindowCreator : public graphics::WindowCreator {
public:
    WindowCreator(const std::shared_ptr<input::Manager> &input_manager);
    ~WindowCreator();

    EGLNativeWindowType create_window(int x, int y, int width, int height) override;
    void destroy_window(EGLNativeWindowType win) override;

    DisplayInfo display_info() const override;
    EGLNativeDisplayType native_display() const override;

private:
    void process_events();
    void process_window_event(const SDL_Event &event);

    std::shared_ptr<input::Manager> input_manager_;
    std::map<EGLNativeWindowType,std::shared_ptr<Window>> windows_;
    std::shared_ptr<Window> current_window_;
    std::thread event_thread_;
    bool event_thread_running_;
};
} // namespace bridge
} // namespace anbox

#endif
