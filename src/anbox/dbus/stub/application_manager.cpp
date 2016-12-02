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

#include "anbox/dbus/stub/application_manager.h"
#include "anbox/dbus/interface.h"
#include "anbox/logger.h"

namespace anbox {
namespace dbus {
namespace stub {
std::shared_ptr<ApplicationManager> ApplicationManager::create_for_bus(
    const core::dbus::Bus::Ptr &bus) {
  auto service = core::dbus::Service::use_service(
      bus, anbox::dbus::interface::Service::name());
  auto object =
      service->add_object_for_path(anbox::dbus::interface::Service::path());
  return std::make_shared<ApplicationManager>(bus, service, object);
}

ApplicationManager::ApplicationManager(const core::dbus::Bus::Ptr &bus,
                                       const core::dbus::Service::Ptr &service,
                                       const core::dbus::Object::Ptr &object)
    : bus_(bus), service_(service), object_(object) {}

ApplicationManager::~ApplicationManager() {}

void ApplicationManager::launch(const android::Intent &intent) {
  auto result = object_->invoke_method_synchronously<
      anbox::dbus::interface::ApplicationManager::Methods::Launch,
      anbox::dbus::interface::ApplicationManager::Methods::Launch::ResultType>(
      intent.action, intent.uri, intent.type, intent.flags, intent.package,
      intent.component);

  if (result.is_error()) throw std::runtime_error(result.error().print());
}
}  // namespace skeleton
}  // namespace dbus
}  // namespace anbox
