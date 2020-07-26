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

#ifndef ANBOX_QEMU_SENSORS_MESSAGE_PROCESSOR_H_
#define ANBOX_QEMU_SENSORS_MESSAGE_PROCESSOR_H_

#include "anbox/application/sensors_state.h"
#include "anbox/qemu/qemud_message_processor.h"

namespace anbox {
namespace qemu {
class SensorsMessageProcessor : public QemudMessageProcessor {
 public:
  SensorsMessageProcessor(
      std::shared_ptr<network::SocketMessenger> messenger, std::shared_ptr<application::SensorsState> sensorsState);
  ~SensorsMessageProcessor();

 protected:
  void handle_command(const std::string& command) override;

 private:
  void send_message(const std::string& message);
  std::shared_ptr<application::SensorsState> sensors_state_;
  std::atomic<int> delay_ = 200;
  std::atomic<bool> temperature_ = false;
  std::atomic<bool> run_thread_ = true;
  std::thread thread_;
  enum SensorType {
    TemperatureSensor = (1 << 3),
  };
};
}  // namespace qemu
}  // namespace anbox

#endif
