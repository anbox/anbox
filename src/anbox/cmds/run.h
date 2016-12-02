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

namespace anbox {
namespace cmds {
class Run : public cli::CommandWithFlagsAndAction {
 public:
  typedef std::function<core::dbus::Bus::Ptr()> BusFactory;

  static BusFactory session_bus_factory();

  Run(const BusFactory& bus_factory = session_bus_factory());

 private:
  BusFactory bus_factory_;
  std::string desktop_file_hint_;
  std::string icon_;
};
}  // namespace cmds
}  // namespace anbox

#endif
