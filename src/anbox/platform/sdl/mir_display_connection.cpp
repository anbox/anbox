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
#include "anbox/platform/sdlmir_display_connection.h"
#include "anbox/logger.h"

#include <boost/throw_exception.hpp>

#include <stdexcept>
#include <string>

namespace {
static const MirDisplayOutput *find_active_output(
    const MirDisplayConfiguration *conf) {
  const MirDisplayOutput *output = NULL;
  int d;

  for (d = 0; d < (int)conf->num_outputs; d++) {
    const MirDisplayOutput *out = conf->outputs + d;

    if (out->used && out->connected && out->num_modes &&
        out->current_mode < out->num_modes) {
      output = out;
      break;
    }
  }

  return output;
}
}

namespace anbox {
namespace sdl {
MirDisplayConnection::MirDisplayConnection()
    : connection_(nullptr),
      output_id_(-1),
      vertical_resolution_(0),
      horizontal_resolution_(0) {
  auto xdg_runtime_dir = ::getenv("XDG_RUNTIME_DIR");
  if (!xdg_runtime_dir)
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to find XDG_RUNTIME_DIR"));

  std::string socket_path = xdg_runtime_dir;
  socket_path += "/mir_socket";

  connection_ = mir_connect_sync(socket_path.c_str(), "anbox");
  if (!mir_connection_is_valid(connection_)) {
    std::string msg;
    msg += "Failed to connect with Mir server: ";
    msg += mir_connection_get_error_message(connection_);
    BOOST_THROW_EXCEPTION(std::runtime_error(msg.c_str()));
  }

  mir_connection_set_display_config_change_callback(
      connection_,
      [](MirConnection *connection, void *context) {
        auto thiz = reinterpret_cast<MirDisplayConnection *>(context);
        DEBUG("");
      },
      this);

  MirDisplayConfiguration *display_config =
      mir_connection_create_display_config(connection_);

  const MirDisplayOutput *output = find_active_output(display_config);
  if (!output)
    BOOST_THROW_EXCEPTION(
        std::runtime_error("Failed to find active output display"));

  DEBUG("Selecting output id %d", output->output_id);

  output_id_ = output->output_id;

  const MirDisplayMode *mode = &output->modes[output->current_mode];

  vertical_resolution_ = mode->vertical_resolution;
  horizontal_resolution_ = mode->horizontal_resolution;

  mir_display_config_destroy(display_config);
}

MirDisplayConnection::~MirDisplayConnection() {
  mir_connection_release(connection_);
}

MirConnection *MirDisplayConnection::connection() const { return connection_; }

MirPixelFormat MirDisplayConnection::default_pixel_format() const {
  MirPixelFormat format;
  unsigned int nformats;
  mir_connection_get_available_surface_formats(connection_, &format, 1,
                                               &nformats);
  return format;
}

EGLNativeDisplayType MirDisplayConnection::native_display() const {
  return mir_connection_get_egl_native_display(connection_);
}

int MirDisplayConnection::output_id() const { return output_id_; }

int MirDisplayConnection::vertical_resolution() const {
  return vertical_resolution_;
}

int MirDisplayConnection::horizontal_resolution() const {
  return horizontal_resolution_;
}
}  // namespace sdl
}  // namespace anbox
