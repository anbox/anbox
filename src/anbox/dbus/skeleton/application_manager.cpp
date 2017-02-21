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
#include "anbox/android/intent.h"
#include "anbox/dbus/interface.h"
#include "anbox/logger.h"

#include <core/property.h>

namespace anbox {
namespace dbus {
namespace skeleton {
ApplicationManager::ApplicationManager(
    const core::dbus::Bus::Ptr &bus, const core::dbus::Object::Ptr &object,
    const std::shared_ptr<anbox::ApplicationManager> &impl)
    : bus_(bus), object_(object), impl_(impl),
      properties_{ object_->get_property<anbox::dbus::interface::ApplicationManager::Properties::Ready>() },
      signals_{ object_->get_signal<core::dbus::interfaces::Properties::Signals::PropertiesChanged>() } {

  object_->install_method_handler<anbox::dbus::interface::ApplicationManager::Methods::Launch>(
      [this](const core::dbus::Message::Ptr &msg) {
        auto reader = msg->reader();

        android::Intent intent;
        reader >> intent.action;
        reader >> intent.uri;
        reader >> intent.type;
        reader >> intent.flags;
        reader >> intent.package;
        reader >> intent.component;

        std::int32_t left, top, right, bottom;
        reader >> left;
        reader >> top;
        reader >> right;
        reader >> bottom;
        graphics::Rect launch_bounds{left, top, right, bottom};

        core::dbus::Message::Ptr reply;

        try {
          launch(intent, launch_bounds);
          reply = core::dbus::Message::make_method_return(msg);
        } catch (std::exception const &err) {
          reply = core::dbus::Message::make_error(msg, "org.anbox.Error.Failed",
                                                  err.what());
        }

        bus_->send(reply);
      });

  // Forward AndroidApi status to our dbus property
  properties_.ready->install([&]() { return impl_->ready().get(); });
  impl_->ready().changed().connect([&](bool value) {
    properties_.ready->set(value);
    on_property_value_changed<anbox::dbus::interface::ApplicationManager::Properties::Ready>(value);
  });
}

ApplicationManager::~ApplicationManager() {}

template<typename Property>
void ApplicationManager::on_property_value_changed(const typename Property::ValueType& value)
{
  typedef std::map<std::string, core::dbus::types::Variant> Dictionary;

  static const std::vector<std::string> the_empty_list_of_invalidated_properties;

  Dictionary dict; dict[Property::name()] = core::dbus::types::Variant::encode(value);

  signals_.properties_changed->emit(
        std::make_tuple(core::dbus::traits::Service<anbox::dbus::interface::ApplicationManager>::interface_name(),
                        dict, the_empty_list_of_invalidated_properties));
}

void ApplicationManager::launch(const android::Intent &intent, const graphics::Rect &launch_bounds) {
  impl_->launch(intent, launch_bounds);
}

core::Property<bool>& ApplicationManager::ready() {
  return impl_->ready();
}
}  // namespace skeleton
}  // namespace dbus
}  // namespace anbox
