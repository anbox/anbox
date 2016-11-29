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
#include "anbox/logger.h"

namespace anbox {
namespace dbus {
namespace skeleton {
ApplicationManager::ApplicationManager(const core::dbus::Bus::Ptr &bus,
                                       const core::dbus::Object::Ptr& object,
                                       const std::shared_ptr<anbox::ApplicationManager> &impl) :
    bus_(bus),
    object_(object),
    impl_(impl) {

    object_->install_method_handler<anbox::dbus::interface::ApplicationManager::Methods::Install>(
                [this](const core::dbus::Message::Ptr &msg) {
        std::string path;
        auto reader = msg->reader();
        reader >> path;

        core::dbus::Message::Ptr reply;

        try {
            install(path);
            DEBUG("install done");
            reply = core::dbus::Message::make_method_return(msg);
            DEBUG("Successfully installed application");
        }
        catch (std::exception const &err) {
            DEBUG("Failed to install application: %s", err.what());
            reply = core::dbus::Message::make_error(msg,
                                                    "org.anbox.Error.Failed",
                                                    err.what());
        }

        bus_->send(reply);
    });

    object_->install_method_handler<anbox::dbus::interface::ApplicationManager::Methods::Launch>(
                [this](const core::dbus::Message::Ptr &msg) {
        std::string package, activity;
        auto reader = msg->reader();
        reader >> package;
        reader >> activity;

        core::dbus::Message::Ptr reply;

        try {
            launch(package, activity);
            reply = core::dbus::Message::make_method_return(msg);
        }
        catch (std::exception const &err) {
            reply = core::dbus::Message::make_error(msg,
                                                    "org.anbox.Error.Failed",
                                                    err.what());
        }

        bus_->send(reply);
    });
}

ApplicationManager::~ApplicationManager() {
    object_->uninstall_method_handler<anbox::dbus::interface::ApplicationManager::Methods::Install>();
}

void ApplicationManager::install(const std::string &path) {
    impl_->install(path);
}

void ApplicationManager::launch(const std::string &package, const std::string &activity) {
    impl_->launch(package, activity);
}
} // namespace skeleton
} // namespace dbus
} // namespace anbox
