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

namespace anbox {
namespace dbus {
namespace skeleton {
ApplicationManager::ApplicationManager(
    const core::dbus::Bus::Ptr &bus, const core::dbus::Object::Ptr &object,
    const std::shared_ptr<anbox::ApplicationManager> &impl)
    : bus_(bus), object_(object), impl_(impl) {
  object_->install_method_handler<
      anbox::dbus::interface::ApplicationManager::Methods::Launch>(
      [this](const core::dbus::Message::Ptr &msg) {
        auto reader = msg->reader();

        android::Intent intent;
        reader >> intent.action;
        reader >> intent.uri;
        reader >> intent.type;
        reader >> intent.flags;
        reader >> intent.package;
        reader >> intent.component;

        core::dbus::Message::Ptr reply;

        try {
          launch(intent);
          reply = core::dbus::Message::make_method_return(msg);
        } catch (std::exception const &err) {
          reply = core::dbus::Message::make_error(msg, "org.anbox.Error.Failed",
                                                  err.what());
        }

        bus_->send(reply);
      });
}

ApplicationManager::~ApplicationManager() {}

void ApplicationManager::launch(const android::Intent &intent) {
  impl_->launch(intent);
}
}  // namespace skeleton
}  // namespace dbus
}  // namespace anbox
