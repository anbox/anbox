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

#ifndef ANBOX_GRAPHICS_MIR_WINDOW_H_
#define ANBOX_GRAPHICS_MIR_WINDOW_H_

#define MIR_EGL_PLATFORM

#include <mirclient/mir_toolkit/mir_client_library.h>

#include <EGL/egl.h>

#include <memory>

namespace anbox {
class InputChannel;

namespace graphics {
class MirDisplayConnection;

class MirWindow {
public:
    MirWindow(const std::shared_ptr<MirDisplayConnection> &display, const std::shared_ptr<InputChannel> &input_channel);
    ~MirWindow();

    EGLNativeWindowType native_window() const;

private:
    static void handle_surface_event(MirSurface *surface, MirEvent const* event, void *context);

    void handle_input_event(MirInputEvent const* input_event);
    void handle_touch_event(MirTouchEvent const* touch_event);

    std::shared_ptr<InputChannel> input_channel_;
    EGLNativeWindowType native_window_;
    MirSurface *surface_;
};

} // namespace graphics
} // namespace anbox

#endif
