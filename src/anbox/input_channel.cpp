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

#include "anbox/input_channel.h"
#include "anbox/logger.h"

#include <boost/throw_exception.hpp>

#include <stdexcept>

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

namespace anbox {
InputChannel::InputChannel() :
    dev_(nullptr) {
}

InputChannel::~InputChannel() {
    if (dev_)
        libevdev_uinput_destroy(dev_);
}

void InputChannel::setup(int width, int height) {
    DEBUG("");
    auto dev = libevdev_new();
    libevdev_set_name(dev, "anbox-tp");

    libevdev_enable_event_type(dev, EV_ABS);
    struct input_absinfo mt_tracking_id = { 0, 0, 255, 0, 0, 0 };
    libevdev_enable_event_code(dev, EV_ABS, ABS_MT_TRACKING_ID, &mt_tracking_id);
    struct input_absinfo mt_slot = { 0, 0, 255, 0, 0, 0 };
    libevdev_enable_event_code(dev, EV_ABS, ABS_MT_SLOT, &mt_slot);
    struct input_absinfo mt_pos_x = { 0, 0, width, 0, 0, 0 };
    libevdev_enable_event_code(dev, EV_ABS, ABS_MT_POSITION_X, &mt_pos_x);
    struct input_absinfo mt_pos_y = { 0, 0, height, 0, 0, 0 };
    libevdev_enable_event_code(dev, EV_ABS, ABS_MT_POSITION_Y, &mt_pos_y);
    struct input_absinfo mt_pressure = { 0, 0, 5, 0, 0, 0 };
    libevdev_enable_event_code(dev, EV_ABS, ABS_MT_PRESSURE, &mt_pressure);

    libevdev_enable_event_type(dev, EV_SYN);

    libevdev_enable_property(dev, INPUT_PROP_DIRECT);

    if (libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &dev_) < 0) {
        libevdev_free(dev);
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create uinput based device"));
    }
}

void InputChannel::push_event(const Event &event) {
    libevdev_uinput_write_event(dev_, event.type, event.code, event.value);
}

std::string InputChannel::dev_path() const {
    if (!dev_)
        return "";

    return std::string(libevdev_uinput_get_devnode(dev_));
}
} // namespace anbox
