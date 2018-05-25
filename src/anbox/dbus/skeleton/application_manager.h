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

#ifndef ANBOX_DBUS_SKELETON_APPLICATION_MANAGER_H_
#define ANBOX_DBUS_SKELETON_APPLICATION_MANAGER_H_

#include "anbox/application/manager.h"
#include "anbox/dbus/bus.h"

#include <memory>

namespace anbox {
namespace dbus {
namespace skeleton {
class ApplicationManager : public anbox::application::Manager {
 public:
  ApplicationManager(const BusPtr& bus, const std::shared_ptr<anbox::application::Manager> &impl);
  ~ApplicationManager();

  void launch(const android::Intent &intent,
              const graphics::Rect &launch_bounds = graphics::Rect::Invalid,
              const wm::Stack::Id &stack = wm::Stack::Id::Default) override;

  core::Property<bool>& ready() override;

 private:
  static const sd_bus_vtable vtable[];
  static int method_launch(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
  static int property_ready_get(sd_bus *bus, const char *path, const char *interface,
                                const char *property, sd_bus_message *reply, void *userdata,
                                sd_bus_error *ret_error);

  BusPtr bus_;
  std::shared_ptr<anbox::application::Manager> impl_;
  sd_bus_slot *obj_slot_ = nullptr;
};
}  // namespace skeleton
}  // namespace dbus
}  // namespace anbox

#endif
