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

#include "anbox/qemu/telephony_manager.h"
#include "anbox/dbus/ofono.h"
#include "anbox/logger.h"

namespace anbox {
namespace qemu {
TelephonyManager::TelephonyManager(const core::dbus::Bus::Ptr &bus)
    : bus_(bus) {
  ofono_ = core::dbus::Service::use_service(bus_, "org.ofono");
  modem_ = ofono_->object_for_path({"/ril_0"});

  auto netreg_prop_changed = modem_->get_signal<
      org::ofono::NetworkRegistration::Signals::PropertyChanged>();
  netreg_prop_changed->connect(
      [&](const org::ofono::NetworkRegistration::Signals::PropertyChanged::
              ArgumentType &arguments) {
        DEBUG("org::ofono::NetworkRegistration::PropertyChanged");
      });
}

TelephonyManager::~TelephonyManager() {}
}  // namespace qemu
}  // namespace anbox
