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

#include "anbox/dbus/interface.h"
#include "anbox/dbus/skeleton/service.h"
#include "anbox/dbus/skeleton/application_manager.h"
#include "anbox/logger.h"

namespace anbox {
namespace dbus {
namespace skeleton {
std::shared_ptr<Service> Service::create_for_bus(const BusPtr& bus, const std::shared_ptr<anbox::application::Manager> &impl) {
  return std::shared_ptr<Service>(new Service(bus, impl));
}

Service::Service(const BusPtr& bus, const std::shared_ptr<anbox::application::Manager> &impl)
    : bus_{bus} {
  if (!bus_)
    throw std::invalid_argument("Missing bus object");

  const auto r = sd_bus_request_name(bus_->raw(),
                                     interface::Service::name(),
                                     0);
  if (r < 0)
    throw std::runtime_error("Failed to request DBus service name");

  DEBUG("Successfully acquired DBus service name");

  application_manager_ = std::make_shared<ApplicationManager>(bus, impl);
}

Service::~Service() {}
}  // namespace skeleton
}  // namespace dbus
}  // namespace anbox
