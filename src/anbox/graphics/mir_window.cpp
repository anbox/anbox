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

#include "anbox/graphics/mir_window.h"
#include "anbox/graphics/mir_display_connection.h"
#include "anbox/logger.h"
#include "anbox/input_channel.h"

namespace anbox {
namespace graphics {
MirWindow::MirWindow(const std::shared_ptr<MirDisplayConnection> &display, const std::shared_ptr<InputChannel> &input_channel) :
    input_channel_(input_channel),
    native_window_(0),
    surface_(nullptr) {

    const auto pixel_format = display->default_pixel_format();

    DEBUG("Selected pixel format %i", pixel_format);

    auto spec = mir_connection_create_spec_for_normal_surface(
                display->connection(),
                display->vertical_resolution(),
                display->horizontal_resolution(),
                pixel_format);

    mir_surface_spec_set_name(spec, "anbox");
    mir_surface_spec_set_event_handler(spec, handle_surface_event, this);
    mir_surface_spec_set_fullscreen_on_output(spec, display->output_id());
    mir_surface_spec_set_buffer_usage(spec, mir_buffer_usage_hardware);

    surface_ = mir_surface_create_sync(spec);
    mir_surface_spec_release(spec);

    if (!mir_surface_is_valid(surface_)) {
        DEBUG("surface error: %s", mir_surface_get_error_message(surface_));
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create a Mir surface"));
    }

    MirSurfaceParameters parameters;
    mir_surface_get_parameters(surface_, &parameters);
    DEBUG("width %i height %i output id %i",
        parameters.width,
        parameters.height,
        parameters.output_id);

    auto surface_buffer_stream = mir_surface_get_buffer_stream(surface_);
    native_window_ = reinterpret_cast<EGLNativeWindowType>(
                mir_buffer_stream_get_egl_native_window(surface_buffer_stream));
}

MirWindow::~MirWindow() {
}

void MirWindow::handle_touch_event(MirTouchEvent const* touch_event) {
    const auto action = mir_touch_event_action(touch_event, 0);
}

void MirWindow::handle_input_event(MirInputEvent const* input_event) {
    const auto type = mir_input_event_get_type(input_event);
    MirTouchEvent const* touch_event = nullptr;
    MirKeyboardEvent const* key_event = nullptr;

    switch (type) {
    case mir_input_event_type_touch:
        touch_event = mir_input_event_get_touch_event(input_event);
        handle_touch_event(touch_event);
        break;
    case mir_input_event_type_pointer:
        break;
    case mir_input_event_type_key:
        key_event = mir_input_event_get_keyboard_event(input_event);
        break;
    }
}

void MirWindow::handle_surface_event(MirSurface *surface, MirEvent const* event, void *context) {
    const auto event_type = mir_event_get_type(event);
    auto thiz = static_cast<MirWindow*>(context);

    switch (event_type) {
    case mir_event_type_input:
        thiz->handle_input_event(mir_event_get_input_event(event));
        break;
    default:
        break;
    }
}

EGLNativeWindowType MirWindow::native_window() const {
    return native_window_;
}
} // namespace graphics
} // namespace anbox
