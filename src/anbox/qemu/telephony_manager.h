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

#ifndef ANBOX_QEMU_TELEPHONY_MANAGER_H_
#define ANBOX_QEMU_TELEPHONY_MANAGER_H_

#include <core/dbus/bus.h>
#include <core/dbus/object.h>
#include <core/dbus/service.h>

namespace anbox {
namespace qemu {
class TelephonyManager {
 public:
  TelephonyManager(const core::dbus::Bus::Ptr &bus);
  ~TelephonyManager();

 private:
  core::dbus::Bus::Ptr bus_;
  core::dbus::Service::Ptr ofono_;
  core::dbus::Object::Ptr modem_;
};
}  // namespace qemu
}  // namespace anbox

#endif
