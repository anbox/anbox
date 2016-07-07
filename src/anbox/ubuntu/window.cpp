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
#include "anbox/ubuntu/mir_display_connection.h"
#include "anbox/input/manager.h"
#include "anbox/input/device.h"
#include "anbox/logger.h"

#include <boost/throw_exception.hpp>

namespace {
class SlotFingerMapper {
public:
    int find_or_create(int id) {
        auto iter = std::find(slots_.begin(), slots_.end(), id);
        if (iter != slots_.end())
            return std::distance(slots_.begin(), iter);

        std::vector<int>::size_type i;
        for(i = 0;i < slots_.size();i++) {
            if (slots_[i] == -1) {
                slots_[i] = id;
                return i;
            }
        }

        slots_.resize(slots_.size() + 1);
        slots_[slots_.size() - 1] = id;

        return slots_.size() - 1;
    }

    void erase(int id)
    {
        auto it = std::find(slots_.begin(), slots_.end(), id);

        if (it != slots_.end()) {
            auto index = std::distance(slots_.begin(), it);
            slots_[index] = -1;
            if (index == slots_.size()-1) {
                while (slots_[index] == -1) {
                    slots_.resize(index);
                    index--;
                }
            }
        }
    }

private:
    std::vector<int> slots_;
};
}


namespace anbox {
namespace ubuntu {
Window::Window(const std::shared_ptr<MirDisplayConnection> &display,
               const std::shared_ptr<input::Manager> &input_manager,
               int width, int height) :
    native_window_(0),
    surface_(nullptr) {

    const auto pixel_format = display->default_pixel_format();

    DEBUG("Selected pixel format %i", pixel_format);

    auto spec = mir_connection_create_spec_for_normal_surface(
                display->connection(),
                width,
                height,
                pixel_format);

    mir_surface_spec_set_name(spec, "anbox");
    mir_surface_spec_set_event_handler(spec, handle_surface_event, this);
    mir_surface_spec_set_fullscreen_on_output(spec, display->output_id());
    mir_surface_spec_set_buffer_usage(spec, mir_buffer_usage_hardware);
    mir_surface_spec_set_shell_chrome(spec, mir_shell_chrome_normal);

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

    // We have to tell mir that we want a maximized window otherwise we
    // get a full screen one.
    mir_surface_set_state(surface_, mir_surface_state_maximized);

    auto surface_buffer_stream = mir_surface_get_buffer_stream(surface_);
    native_window_ = reinterpret_cast<EGLNativeWindowType>(
                mir_buffer_stream_get_egl_native_window(surface_buffer_stream));

    // Create our touch panel input device
    touchpanel_ = input_manager->create_device();
    touchpanel_->set_name("mir-touchpanel");
    touchpanel_->set_driver_version(1);
    touchpanel_->set_input_id({ BUS_VIRTUAL, 1, 1, 1 });
    touchpanel_->set_physical_location("none");
    touchpanel_->set_abs_bit(ABS_MT_TRACKING_ID);
    touchpanel_->set_abs_bit(ABS_MT_SLOT);
    touchpanel_->set_abs_bit(ABS_MT_POSITION_X);
    touchpanel_->set_abs_bit(ABS_MT_POSITION_Y);
    touchpanel_->set_prop_bit(INPUT_PROP_DIRECT);
    touchpanel_->set_abs_min(ABS_MT_POSITION_X, 0);
    touchpanel_->set_abs_max(ABS_MT_POSITION_X, parameters.width);
    touchpanel_->set_abs_min(ABS_MT_POSITION_Y, 0);
    touchpanel_->set_abs_max(ABS_MT_POSITION_Y, parameters.height);
    touchpanel_->set_abs_max(ABS_MT_SLOT, 10);
    touchpanel_->set_abs_max(ABS_MT_TRACKING_ID, 255);

    pointer_ = input_manager->create_device();
    pointer_->set_name("mir-pointer");
    pointer_->set_driver_version(1);
    pointer_->set_input_id({ BUS_VIRTUAL, 1, 1, 1 });
    pointer_->set_physical_location("none");
#if 0
    pointer_->set_key_bit(BTN_MOUSE);
    pointer_->set_key_bit(BTN_LEFT);
    pointer_->set_key_bit(BTN_MIDDLE);
    pointer_->set_key_bit(BTN_RIGHT);
#endif
    pointer_->set_rel_bit(REL_X);
    pointer_->set_rel_bit(REL_Y);
    pointer_->set_rel_bit(REL_HWHEEL);
    pointer_->set_rel_bit(REL_WHEEL);
    pointer_->set_prop_bit(INPUT_PROP_POINTER);
}

Window::~Window() {
}

void Window::handle_touch_event(MirTouchEvent const* touch_event) {
    const auto point_count = mir_touch_event_point_count(touch_event);

    static SlotFingerMapper mapper;

    for (size_t n = 0; n < point_count; n++) {
        const auto id = mir_touch_event_id(touch_event, n);
        const auto slot = mapper.find_or_create(id);
        const auto x = mir_touch_event_axis_value(touch_event, n, mir_touch_axis_x);
        const auto y = mir_touch_event_axis_value(touch_event, n, mir_touch_axis_y);
        const auto touch_major = mir_touch_event_axis_value(touch_event, n, mir_touch_axis_touch_major);
        const auto touch_minor = mir_touch_event_axis_value(touch_event, n, mir_touch_axis_touch_minor);

        std::vector<input::Event> events;

        switch (mir_touch_event_action(touch_event, n)) {
        case mir_touch_action_up:
            events.push_back({ EV_ABS, ABS_MT_SLOT, slot });
            events.push_back({ EV_ABS, ABS_MT_TRACKING_ID, -1 });
            events.push_back({ EV_KEY, BTN_TOUCH, 0 });
            events.push_back({ EV_KEY, BTN_TOOL_FINGER, 0 });
            events.push_back({ EV_SYN, SYN_REPORT, 0 });
            mapper.erase(id);
            break;
        case mir_touch_action_down:
            events.push_back({ EV_ABS, ABS_MT_SLOT, slot });
            events.push_back({ EV_ABS, ABS_MT_TRACKING_ID, id });
            events.push_back({ EV_ABS, ABS_MT_POSITION_X, static_cast<std::int32_t>(x) });
            events.push_back({ EV_ABS, ABS_MT_POSITION_Y, static_cast<std::int32_t>(y) });
            events.push_back({ EV_ABS, ABS_MT_TOUCH_MAJOR, static_cast<std::int32_t>(touch_major) });
            events.push_back({ EV_ABS, ABS_MT_TOUCH_MINOR, static_cast<std::int32_t>(touch_minor) });
            events.push_back({ EV_SYN, SYN_REPORT, 0 });
            break;
        case mir_touch_action_change:
            events.push_back({ EV_ABS, ABS_MT_SLOT, slot });
            events.push_back({ EV_ABS, ABS_MT_TRACKING_ID, id });
            events.push_back({ EV_KEY, BTN_TOUCH, 1 });
            events.push_back({ EV_KEY, BTN_TOOL_FINGER, 1 });
            events.push_back({ EV_ABS, ABS_MT_POSITION_X, static_cast<std::int32_t>(x) });
            events.push_back({ EV_ABS, ABS_MT_POSITION_Y, static_cast<std::int32_t>(y) });
            events.push_back({ EV_ABS, ABS_MT_TOUCH_MAJOR, static_cast<std::int32_t>(touch_major) });
            events.push_back({ EV_ABS, ABS_MT_TOUCH_MINOR, static_cast<std::int32_t>(touch_minor) });
            events.push_back({ EV_SYN, SYN_REPORT, 0 });
            break;
        }

        touchpanel_->send_events(events);
    }
}

void Window::handle_pointer_button_event(std::vector<input::Event> &events,
                                         MirPointerEvent const* pointer_event, bool pressed) {
    static uint32_t old_button_states = 0;
    uint32_t new_button_states = mir_pointer_event_buttons(pointer_event);

    MirPointerButton button_state = mir_pointer_button_primary;
    button_state = static_cast<MirPointerButton>(new_button_states ^ old_button_states);

    std::uint16_t button;

    switch (button_state) {
    case mir_pointer_button_primary:
        button = BTN_LEFT;
        break;
    case mir_pointer_button_secondary:
        button = BTN_RIGHT;
        break;
    case mir_pointer_button_tertiary:
        button = BTN_MIDDLE;
        break;
    default:
        break;
    }

    events.push_back({ EV_KEY, button, pressed ? 1 : 0 });
    events.push_back({ EV_SYN, SYN_REPORT, 0 });
}

void Window::handle_pointer_event(MirPointerEvent const* pointer_event) {
    std::vector<input::Event> events;
    static MirPointerButtons button_state = 0;

    const float x = mir_pointer_event_axis_value(pointer_event, mir_pointer_axis_x);
    const float y = mir_pointer_event_axis_value(pointer_event, mir_pointer_axis_y);

    switch (mir_pointer_event_action(pointer_event)) {
    case mir_pointer_action_button_down:
        handle_pointer_button_event(events, pointer_event, true);
        break;
    case mir_pointer_action_button_up:
        handle_pointer_button_event(events, pointer_event, false);
        break;
    case mir_pointer_action_motion:
        {
            const float hdelta = mir_pointer_event_axis_value(pointer_event, mir_pointer_axis_hscroll);
            const float vdelta = mir_pointer_event_axis_value(pointer_event, mir_pointer_axis_vscroll);
            if (hdelta != 0.0 || vdelta != 0.0) {
                events.push_back({ EV_REL, REL_HWHEEL, static_cast<std::int32_t>(hdelta) });
                events.push_back({ EV_REL, REL_WHEEL, static_cast<std::int32_t>(vdelta) });
                events.push_back({ EV_SYN, SYN_REPORT, 0 });
            }

            const float xdelta = mir_pointer_event_axis_value(pointer_event, mir_pointer_axis_relative_x);
            const float ydelta = mir_pointer_event_axis_value(pointer_event, mir_pointer_axis_relative_y);
            if (xdelta != 0.0 || ydelta != 0.0) {
                events.push_back({ EV_REL, REL_X, static_cast<std::int32_t>(xdelta) });
                events.push_back({ EV_REL, REL_Y, static_cast<std::int32_t>(ydelta) });
                events.push_back({ EV_SYN, SYN_REPORT, 0 });
            }
        }
        break;
    default:
        break;
    }

    pointer_->send_events(events);
}

void Window::handle_input_event(MirInputEvent const* input_event) {
    const auto type = mir_input_event_get_type(input_event);
    MirTouchEvent const* touch_event = nullptr;
    MirKeyboardEvent const* key_event = nullptr;
    MirPointerEvent const* pointer_event = nullptr;

    switch (type) {
    case mir_input_event_type_touch:
        touch_event = mir_input_event_get_touch_event(input_event);
        handle_touch_event(touch_event);
        break;
    case mir_input_event_type_pointer:
#if 0
        pointer_event = mir_input_event_get_pointer_event(input_event);
        handle_pointer_event(pointer_event);
#endif
        break;
    case mir_input_event_type_key:
        key_event = mir_input_event_get_keyboard_event(input_event);
        break;
    }
}

void Window::handle_surface_event(MirSurface*, MirEvent const* event, void *context) {
    const auto event_type = mir_event_get_type(event);
    auto thiz = static_cast<Window*>(context);

    switch (event_type) {
    case mir_event_type_input:
        thiz->handle_input_event(mir_event_get_input_event(event));
        break;
    default:
        break;
    }
}

EGLNativeWindowType Window::native_window() const {
    return native_window_;
}
} // namespace bridge
} // namespace anbox
