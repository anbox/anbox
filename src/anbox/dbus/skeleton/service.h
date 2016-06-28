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

#ifndef ANBOX_DBUS_SKELETON_SERVICE_H_
#define ANBOX_DBUS_SKELETON_SERVICE_H_

#include "anbox/do_not_copy_or_move.h"
#include "anbox/application_manager.h"

#include <core/dbus/bus.h>
#include <core/dbus/object.h>
#include <core/dbus/service.h>

namespace anbox {
namespace dbus {
namespace skeleton {
class ApplicationManager;
class Service : public DoNotCopyOrMove {
public:
    static std::shared_ptr<Service> create_for_bus(const core::dbus::Bus::Ptr &bus,
                                                   const std::shared_ptr<anbox::ApplicationManager> &application_manager);

    Service(const core::dbus::Bus::Ptr &bus,
            const core::dbus::Service::Ptr& service,
            const core::dbus::Object::Ptr& object,
            const std::shared_ptr<anbox::ApplicationManager> &application_manager);
    ~Service();

private:
    core::dbus::Bus::Ptr bus_;
    core::dbus::Service::Ptr service_;
    core::dbus::Object::Ptr object_;
    std::shared_ptr<ApplicationManager> application_manager_;
};
} // namespace skeleton
} // namespace dbus
} // namespace anbox

#endif
