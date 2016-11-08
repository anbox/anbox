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
namespace ubuntu {
class Window {
public:
    Window(int x, int y, int width, int height);
    ~Window();

    void resize(int width, int height);
    int update_position(int x, int y);

    EGLNativeWindowType native_window() const;

private:
    EGLNativeDisplayType native_display_;
    EGLNativeWindowType native_window_;
    SDL_Window *window_;
};
} // namespace bridge
} // namespace anbox

#endif
