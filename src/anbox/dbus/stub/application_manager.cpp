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
#include "anbox/dbus/stub/application_manager.h"
#include "anbox/logger.h"

#include <sstream>

namespace anbox {
namespace dbus {
namespace stub {
std::shared_ptr<ApplicationManager> ApplicationManager::create_for_bus(const BusPtr& bus) {
  return std::shared_ptr<ApplicationManager>(new ApplicationManager(bus));
}

ApplicationManager::ApplicationManager(const BusPtr& bus)
    : bus_(bus) {

  if (!bus_->has_service_with_name(interface::Service::name()))
    throw std::runtime_error("Application manager service is not running yet");

  update_properties();
}

ApplicationManager::~ApplicationManager() {}

void ApplicationManager::update_properties() {
  int ready = 0;
  const auto r = sd_bus_get_property_trivial(bus_->raw(),
                                             interface::Service::name(),
                                             interface::Service::path(),
                                             interface::ApplicationManager::name(),
                                             interface::ApplicationManager::Properties::Ready::name(),
                                             nullptr,
                                             'b',
                                             &ready);
  if (r < 0)
    throw std::runtime_error("Failed to retrieve ready property from application manager");

  ready_.set(ready);
}

void ApplicationManager::launch(const android::Intent &intent,
                                const graphics::Rect &launch_bounds,
                                const wm::Stack::Id &stack) {
  (void) launch_bounds;

  sd_bus_message *m = nullptr;
  auto r = sd_bus_message_new_method_call(bus_->raw(),
                                          &m,
                                          interface::Service::name(),
                                          interface::Service::path(),
                                          interface::ApplicationManager::name(),
                                          interface::ApplicationManager::Methods::Launch::name());
  if (r < 0)
    throw std::runtime_error("Failed to construct DBus message");

  r = sd_bus_message_open_container(m, 'a', "{sv}");
  if (r < 0)
    throw std::runtime_error("Failed to construct DBus message");

  if (intent.package.length() > 0) {
    r = sd_bus_message_append(m, "{sv}", "package", "s", intent.package.c_str());
    if (r < 0)
      throw std::runtime_error("Failed to construct DBus message");
  }

  if (intent.component.length() > 0) {
    r = sd_bus_message_append(m, "{sv}", "component", "s", intent.component.c_str());
    if (r < 0)
      throw std::runtime_error("Failed to construct DBus message");
  }

  if (intent.action.length() > 0) {
    r = sd_bus_message_append(m, "{sv}", "action", "s", intent.action.c_str());
    if (r < 0)
      throw std::runtime_error("Failed to construct DBus message");
  }

  if (intent.type.length() > 0) {
    r = sd_bus_message_append(m, "{sv}", "type", "s", intent.type.c_str());
    if (r < 0)
      throw std::runtime_error("Failed to construct DBus message");
  }

  if (intent.uri.length() > 0) {
    r = sd_bus_message_append(m, "{sv}", "uri", "s", intent.uri.c_str());
    if (r < 0)
      throw std::runtime_error("Failed to construct DBus message");
  }

  r = sd_bus_message_close_container(m);
  if (r < 0)
    throw std::runtime_error("Failed to construct DBus message");

  std::ostringstream launch_stack;
  launch_stack << stack;
  r = sd_bus_message_append(m, "s", launch_stack.str().c_str());
  if (r < 0)
    throw std::runtime_error("Failed to construct DBus message");

  #pragma GCC diagnostic push
  #pragma GCC diagnostic warning "-Wpragmas"
  #pragma GCC diagnostic warning "-Wc99-extensions"
  sd_bus_error error = SD_BUS_ERROR_NULL;
  #pragma GCC diagnostic pop

  r = sd_bus_call(bus_->raw(), m, 0, &error, nullptr);
  if (r < 0) {
    const auto msg = utils::string_format("%s", error.message);
    sd_bus_error_free(&error);
    throw std::runtime_error(msg);
  }
}

core::Property<bool>& ApplicationManager::ready() {
  return ready_;
}
}  // namespace skeleton
}  // namespace dbus
}  // namespace anbox
