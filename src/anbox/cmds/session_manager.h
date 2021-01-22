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

#pragma once

#include "anbox/cli.h"

#include <functional>
#include <iostream>
#include <memory>

#include "anbox/graphics/gl_renderer_server.h"
#include "anbox/graphics/rect.h"

namespace anbox::bridge {
  class AndroidApiStub;
}

namespace anbox::container {
  class Client;
}

namespace anbox::cmds {
class SessionManager : public cli::CommandWithFlagsAndAction {
 public:
  SessionManager();

 private:
  void launch_appmgr_if_needed(const std::shared_ptr<bridge::AndroidApiStub> &android_api_stub);

  std::shared_ptr<container::Client> container_;
  std::string desktop_file_hint_;
  std::string disabled_sensors_;
  bool single_window_ = false;
  graphics::Rect window_size_;
  bool standalone_ = false;
  bool experimental_ = false;
  bool use_system_dbus_ = false;
  bool use_software_rendering_ = false;
  bool no_touch_emulation_ = false;
  bool server_side_decoration_ = false;
};
}
