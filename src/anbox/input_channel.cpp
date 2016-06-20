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

namespace anbox {
InputChannel::InputChannel() :
    dev_(nullptr) {
}

InputChannel::~InputChannel() {
}

void InputChannel::setup(int width, int height) {
}

void InputChannel::push_event(const Event &event) {
}

std::string InputChannel::dev_path() const {
    return "";
}
} // namespace anbox
