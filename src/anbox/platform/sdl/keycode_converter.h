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

#ifndef ANBOX_PLATFORM_SDL_KEYCODE_CONVERTER_H_
#define ANBOX_PLATFORM_SDL_KEYCODE_CONVERTER_H_

#include "anbox/platform/sdl/sdl_wrapper.h"

#include <cstdint>

#include <array>

namespace anbox {
namespace platform {
namespace sdl {
class KeycodeConverter {
 public:
  static std::uint16_t convert(const SDL_Scancode &scan_code);

 private:
  static const std::array<SDL_Scancode, 249> code_map;
};
} // namespace sdl
} // namespace platform
} // namespace anbox

#endif
