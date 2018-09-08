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

#include "anbox/dbus/bus.h"
#include "anbox/logger.h"

namespace anbox {
namespace dbus {
Bus::Bus(Type type) {
  int ret = 0;
  switch (type) {
  case Type::Session:
    ret = sd_bus_open_user(&bus_);
    break;
  case Type::System:
    ret = sd_bus_open_system(&bus_);
    break;
  default:
    throw std::invalid_argument("Invalid bus type");
  }

  if (ret < 0 || !bus_)
    throw std::runtime_error("Failed to connect to DBus");
}

Bus::~Bus() {
  stop();

  if (bus_)
    sd_bus_unref(bus_);
}

bool Bus::has_service_with_name(const std::string &name) {
  auto r = sd_bus_get_name_creds(bus_,
                                 name.c_str(),
                                 0,
                                 nullptr);
  return r >= 0;
}

sd_bus* Bus::raw() {
  return bus_;
}

void Bus::run_async() {
  running_ = true;
  worker_thread_ = std::thread(&Bus::worker_main, this);
}

void Bus::stop() {
  running_ = false;
  if (worker_thread_.joinable())
    worker_thread_.join();
}

void Bus::worker_main() {
  while (running_) {
    auto ret = sd_bus_process(bus_, nullptr);
    if (ret < 0)
      break;
    if (ret > 0)
      continue;

    ret = sd_bus_wait(bus_, 1000 * 500);
    if (ret < 0)
      break;
  }
}
}  // namespace dbus
}  // namespace anbox
