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

#include <mirclient/mir_toolkit/mir_client_library.h>

namespace anbox {
namespace input {
class Manager;
class Device;
} // namespace input
namespace ubuntu {
class MirDisplayConnection;
class Window {
public:
    Window(const std::shared_ptr<MirDisplayConnection> &display,
           const std::shared_ptr<input::Manager> &input_manager,
           int width, int height);
    ~Window();

    EGLNativeWindowType native_window() const;

private:
    static void handle_surface_event(MirSurface *surface, MirEvent const* event, void *context);

    void handle_input_event(MirInputEvent const* input_event);
    void handle_touch_event(MirTouchEvent const* touch_event);
    void handle_pointer_event(MirPointerEvent const* pointer_event);

    std::shared_ptr<input::Device> touchpanel_;
    std::shared_ptr<input::Device> pointer_;
    EGLNativeWindowType native_window_;
    MirSurface *surface_;
};
} // namespace bridge
} // namespace anbox

#endif
