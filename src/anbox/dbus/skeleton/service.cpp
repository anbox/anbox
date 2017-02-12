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

#include "anbox/dbus/skeleton/service.h"
#include "anbox/dbus/interface.h"
#include "anbox/dbus/skeleton/application_manager.h"

namespace anbox {
namespace dbus {
namespace skeleton {
std::shared_ptr<Service> Service::create_for_bus(
    const core::dbus::Bus::Ptr &bus,
    const std::shared_ptr<anbox::ApplicationManager> &application_manager) {
  auto service = core::dbus::Service::add_service(
      bus, anbox::dbus::interface::Service::name());
  auto object =
      service->add_object_for_path(anbox::dbus::interface::Service::path());
  return std::make_shared<Service>(bus, service, object, application_manager);
}

Service::Service(
    const core::dbus::Bus::Ptr &bus, const core::dbus::Service::Ptr &service,
    const core::dbus::Object::Ptr &object,
    const std::shared_ptr<anbox::ApplicationManager> &application_manager)
    : bus_(bus),
      service_(service),
      object_(object),
      application_manager_(std::make_shared<ApplicationManager>(bus_, object_, application_manager)) {}

Service::~Service() {}
}  // namespace skeleton
}  // namespace dbus
}  // namespace anbox
