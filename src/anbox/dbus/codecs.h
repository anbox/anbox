/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ANBOX_DBUS_CODECS_H_
#define ANBOX_DBUS_CODECS_H_

#include "anbox/wm/stack.h"

#include <core/dbus/codec.h>

#include <sstream>

namespace core {
namespace dbus {
template<>
struct Codec<anbox::wm::Stack::Id> {
  inline static void encode_argument(Message::Writer &out, const anbox::wm::Stack::Id &stack) {
    std::stringstream ss;
    ss << stack;
    auto s = ss.str();
    out.push_stringn(s.c_str(), s.length());
  }

  inline static void decode_argument(Message::Reader &in, anbox::wm::Stack::Id &stack) {
    std::stringstream ss;
    ss << in.pop_string();
    ss >> stack;
  }
};
} // namespace dbus
} // namespace core

#endif
