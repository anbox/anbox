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

#ifndef ANBOX_DBUS_BUS_H_
#define ANBOX_DBUS_BUS_H_

#include "anbox/do_not_copy_or_move.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include <systemd/sd-bus.h>

namespace anbox {
namespace dbus {
class Bus : public DoNotCopyOrMove {
 public:
  enum class Type {
    System,
    Session
  };

  Bus(Type type);
  ~Bus();

  sd_bus* raw();

  bool has_service_with_name(const std::string& name);
  void run_async();
  void stop();

 private:
  void worker_main();

  sd_bus *bus_ = nullptr;
  std::thread worker_thread_;
  std::atomic_bool running_{false};
};
using BusPtr = std::shared_ptr<Bus>;
}  // namespace dbus
}  // namespace anbox

#endif
