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

#ifndef ANBOX_PLATFORM_SDL_MIR_DISPLAY_CONNECTION_H_
#define ANBOX_PLATFORM_SDL_MIR_DISPLAY_CONNECTION_H_

#define MIR_EGL_PLATFORM

#include <mirclient/mir_toolkit/mir_client_library.h>

#include <EGL/egl.h>

namespace anbox {
namespace sdl {
class MirDisplayConnection {
 public:
  MirDisplayConnection();
  ~MirDisplayConnection();

  MirPixelFormat default_pixel_format() const;

  MirConnection* connection() const;
  EGLNativeDisplayType native_display() const;

  int output_id() const;
  int vertical_resolution() const;
  int horizontal_resolution() const;

 private:
  MirConnection* connection_;
  int output_id_;
  int vertical_resolution_;
  int horizontal_resolution_;
};
}  // namespace sdl
}  // namespace anbox

#endif
