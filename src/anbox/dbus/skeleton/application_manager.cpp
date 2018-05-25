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

#include "anbox/dbus/skeleton/application_manager.h"
#include "anbox/dbus/interface.h"
#include "anbox/dbus/sd_bus_helpers.h"
#include "anbox/android/intent.h"
#include "anbox/logger.h"

#include <sstream>

#include <core/property.h>

namespace {
int parse_string_from_message(sd_bus_message *m, std::string &str) {
  const char *contents = nullptr;
  auto r = sd_bus_message_enter_container(m, SD_BUS_TYPE_VARIANT, contents);
  if (r < 0)
    return r;

  const char *value;
  r = sd_bus_message_read(m, "s", &value);
  if (r < 0)
    return r;

  str = value;

  r = sd_bus_message_exit_container(m);
  if (r < 0)
    return r;

  return 0;
}
} // namespace

namespace anbox {
namespace dbus {
namespace skeleton {
const sd_bus_vtable ApplicationManager::vtable[] = {
  sdbus::vtable::start(0),
  sdbus::vtable::method("Launch", "a{sv}s", "", ApplicationManager::method_launch, SD_BUS_VTABLE_UNPRIVILEGED),
  sdbus::vtable::property("Ready", "b", ApplicationManager::property_ready_get, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
  sdbus::vtable::end()
};

int ApplicationManager::method_launch(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
  auto r = sd_bus_message_enter_container(m, SD_BUS_TYPE_ARRAY, "{sv}");
  if (r < 0)
    return r;

  android::Intent intent;

  while ((r = sd_bus_message_enter_container(m, SD_BUS_TYPE_DICT_ENTRY, "sv")) > 0) {
    const char *key = nullptr;

    r = sd_bus_message_read(m, "s", &key);
    if (r < 0)
      return r;

    if (strcmp(key, "package") == 0) {
      r = parse_string_from_message(m, intent.package);
      if (r < 0)
        return r;
    } else if (strcmp(key, "component") == 0) {
      r = parse_string_from_message(m, intent.component);
      if (r < 0)
        return r;
    } else if (strcmp(key, "action") == 0) {
      r = parse_string_from_message(m, intent.action);
      if (r < 0)
        return r;
    } else if (strcmp(key, "type") == 0) {
      r = parse_string_from_message(m, intent.type);
      if (r < 0)
        return r;
    } else if (strcmp(key, "uri") == 0) {
      r = parse_string_from_message(m, intent.uri);
      if (r < 0)
        return r;
    }

    r = sd_bus_message_exit_container(m);
    if (r < 0)
      return r;
  }

  r = sd_bus_message_exit_container(m);
  if (r < 0)
    return r;

  const char *stack = nullptr;
  r = sd_bus_message_read(m, "s", &stack);
  if (r <  0)
    return r;

  wm::Stack::Id launch_stack = wm::Stack::Id::Default;
  if (stack && strlen(stack) > 0) {
    auto s = std::string(stack);
    std::istringstream i(s);
    i >> launch_stack;
  }

  if (intent.package.length() == 0) {
    sd_bus_error_set_const(ret_error, "org.anbox.InvalidArgument", "No package specified");
    return -EINVAL;
  }

  auto thiz = static_cast<ApplicationManager*>(userdata);
  try {
    thiz->launch(intent, graphics::Rect::Invalid, launch_stack);
  } catch (std::exception &err) {
    ERROR("Failed to launch application: %s", err.what());
    sd_bus_error_set_const(ret_error, "org.anbox.InternalError", err.what());
    return -EIO;
  }

  return sd_bus_reply_method_return(m, "");
}

int ApplicationManager::property_ready_get(sd_bus *bus, const char *path, const char *interface,
                                           const char *property, sd_bus_message *reply,
                                           void *userdata, sd_bus_error *ret_error) {

  (void) bus;
  (void) path;
  (void) interface;
  (void) property;
  (void) ret_error;

  auto thiz = static_cast<ApplicationManager*>(userdata);

  return sd_bus_message_append(reply, "b", thiz->impl_->ready().get());
}

ApplicationManager::ApplicationManager(const BusPtr& bus, const std::shared_ptr<anbox::application::Manager> &impl)
    : bus_(bus), impl_(impl) {

  const auto r = sd_bus_add_object_vtable(bus_->raw(),
                                          &obj_slot_,
                                          interface::Service::path(),
                                          interface::ApplicationManager::name(),
                                          vtable,
                                          this);
  if (r < 0)
    std::runtime_error("Failed to setup application manager DBus service");

  impl_->ready().changed().connect([&](bool value) {
    (void) value;

    sd_bus_emit_properties_changed(bus_->raw(),
                                   interface::Service::path(),
                                   interface::ApplicationManager::name(),
                                   interface::ApplicationManager::Properties::Ready::name(),
                                   nullptr);
  });
}

ApplicationManager::~ApplicationManager() {}

void ApplicationManager::launch(const android::Intent &intent,
                                const graphics::Rect &launch_bounds,
                                const wm::Stack::Id &stack) {
  if (!impl_->ready())
    throw std::runtime_error("Anbox not yet ready to launch applications");

  DEBUG("Launching %s", intent);
  impl_->launch(intent, launch_bounds, stack);
}

core::Property<bool>& ApplicationManager::ready() {
  return impl_->ready();
}
}  // namespace skeleton
}  // namespace dbus
}  // namespace anbox
