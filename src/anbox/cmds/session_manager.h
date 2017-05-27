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

#ifndef ANBOX_CMDS_RUN_H_
#define ANBOX_CMDS_RUN_H_

#include "anbox/cli.h"

#include <functional>
#include <iostream>
#include <memory>

#include <core/dbus/bus.h>

#include "anbox/graphics/gl_renderer_server.h"
#include "anbox/graphics/rect.h"

namespace anbox {
namespace container {
class Client;
}  // namespace container
namespace cmds {
class SessionManager : public cli::CommandWithFlagsAndAction {
 public:
  typedef std::function<core::dbus::Bus::Ptr()> BusFactory;

  static BusFactory session_bus_factory();

  SessionManager(const BusFactory& bus_factory = session_bus_factory());

 private:
  std::shared_ptr<container::Client> container_;
  BusFactory bus_factory_;
  std::string desktop_file_hint_;
  graphics::GLRendererServer::Config::Driver gles_driver_;
  bool single_window_ = false;
  graphics::Rect window_size_;
  bool standalone_ = false;
  bool experimental_ = false;
};
}  // namespace cmds
}  // namespace anbox

#endif
