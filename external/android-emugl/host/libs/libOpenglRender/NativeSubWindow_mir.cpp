/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#define MIR_EGL_PLATFORM

#include "mir_support/shared_state.h"

#include "NativeSubWindow.h"

#include "OpenglCodecCommon/ErrorLog.h"

#include <stdexcept>
#include <string>
#include <map>

#include <mirclient/mir_toolkit/mir_client_library.h>

namespace {
std::map<EGLNativeWindowType,MirSurface*> surfaces;

static const MirDisplayOutput *find_active_output(
    const MirDisplayConfiguration *conf)
{
    const MirDisplayOutput *output = NULL;
    int d;

    for (d = 0; d < (int)conf->num_outputs; d++)
    {
        const MirDisplayOutput *out = conf->outputs + d;

        if (out->used &&
            out->connected &&
            out->num_modes &&
            out->current_mode < out->num_modes)
        {
            output = out;
            break;
        }
    }

    return output;
}
}

MirPixelFormat defaultPixelFormatFor(MirConnection *connection)
{
    MirPixelFormat format;
    unsigned int nformats;
    mir_connection_get_available_surface_formats(connection, &format, 1, &nformats);
    return format;
}

void handleSurfaceEvent(const MirSurfaceEvent *event) {
    MirSurfaceAttrib attrib = mir_surface_event_get_attribute(event);
    if (attrib != mir_surface_attrib_visibility)
        return;
    switch (mir_surface_event_get_attribute(event)) {
    case mir_surface_visibility_exposed:
        ERR("Surface exposed\n");
        break;
    case mir_surface_visibility_occluded:
        ERR("Surface occluded\n");
        break;
    default:
        break;
    }
}

void surfaceEventHandler(MirSurface *surface, MirEvent const *event, void *context) {
    switch (mir_event_get_type(event)) {
    case mir_event_type_input:
        break;
    case mir_event_type_surface:
        handleSurfaceEvent(mir_event_get_surface_event(event));
        break;
    case mir_event_type_resize:
        {
            MirResizeEvent const *resize_event = mir_event_get_resize_event(event);
            ERR("Resized to %dx%d\n",
                mir_resize_event_get_width(resize_event),
                mir_resize_event_get_height(resize_event));
        }
        break;
    case mir_event_type_close_surface:
        break;
    case mir_event_type_surface_output:
        {
            MirSurfaceOutputEvent const *output_event = mir_event_get_surface_output_event(event);
            ERR("dpi %i output id %d\n",
                mir_surface_output_event_get_dpi(output_event),
                mir_surface_output_event_get_output_id(output_event));
        }
        break;
    default:
        break;
    }
}

EGLNativeWindowType createSubWindow(FBNativeWindowType p_window,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    SubWindowRepaintCallback repaint_callback,
                                    void* repaint_callback_param) {

    (void) p_window;
    (void) x;
    (void) y;
    (void) width;
    (void) height;
    (void) repaint_callback;
    (void) repaint_callback_param;

    mir::support::SharedState::get()->ensure_connection();

    const auto pixel_format = defaultPixelFormatFor(mir::support::SharedState::get()->connection());

    ERR("Selected pixel format %d\n", pixel_format);

    MirDisplayConfiguration* display_config =
        mir_connection_create_display_config(mir::support::SharedState::get()->connection());

    const MirDisplayOutput *output = find_active_output(display_config);
    if (!output)
        throw new std::runtime_error("Failed to find active output display");

    const MirDisplayMode *mode = &output->modes[output->current_mode];

    auto spec = mir_connection_create_spec_for_normal_surface(
                mir::support::SharedState::get()->connection(),
                mode->vertical_resolution,
                mode->horizontal_resolution,
                pixel_format);

    mir_surface_spec_set_name(spec, "anbox");
    mir_surface_spec_set_event_handler(spec, surfaceEventHandler, nullptr);
    ERR("Selecting output id %d", output->output_id);
    mir_surface_spec_set_fullscreen_on_output(spec, output->output_id);
    mir_surface_spec_set_buffer_usage(spec, mir_buffer_usage_hardware);

    mir_display_config_destroy(display_config);

    auto surface = mir_surface_create_sync(spec);
    mir_surface_spec_release(spec);

    if (!mir_surface_is_valid(surface)) {
        ERR("surface error: %s\n", mir_surface_get_error_message(surface));
        throw std::runtime_error("Failed to create a Mir surface");
    }

    MirSurfaceParameters parameters;
    mir_surface_get_parameters(surface, &parameters);
    ERR("width %i height %i output id %i\n",
        parameters.width,
        parameters.height,
        parameters.output_id);

    auto surface_buffer_stream = mir_surface_get_buffer_stream(surface);
    auto native_window = reinterpret_cast<EGLNativeWindowType>(
                mir_buffer_stream_get_egl_native_window(surface_buffer_stream));

    surfaces.insert({ native_window, surface });

    return native_window;
}

void destroySubWindow(EGLNativeWindowType win) {
    if (surfaces.find(win) == surfaces.end())
        return;

    auto surface = surfaces[win];
    surfaces.erase(win);

    mir_surface_release_sync(surface);
}

int moveSubWindow(FBNativeWindowType p_parent_window,
                  EGLNativeWindowType p_sub_window,
                  int x,
                  int y,
                  int width,
                  int height) {

    ERR("%s: Not implemented", __func__);

    return true;
}
