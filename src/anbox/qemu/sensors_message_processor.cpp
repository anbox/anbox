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

#include "anbox/qemu/sensors_message_processor.h"

#include <boost/algorithm/string/predicate.hpp>
#include <chrono>
#include <iostream>
#include <thread>

#include "anbox/logger.h"

using namespace std;

namespace anbox {
namespace qemu {
SensorsMessageProcessor::SensorsMessageProcessor(
    shared_ptr<network::SocketMessenger> messenger, shared_ptr<application::SensorsState> sensorsState)
    : QemudMessageProcessor(messenger), sensors_state_(sensorsState) {
  thread_ = std::thread([this]() {
    for (;;) {
      if (temperature_.load())
        send_message("temperature:" + to_string(sensors_state_->temperature));
      if (!run_thread_.load())
        break;
      this_thread::sleep_for(delay_.load() * 1ms);
    }
  });
}

SensorsMessageProcessor::~SensorsMessageProcessor() {
  run_thread_ = false;
  thread_.join();
}

void SensorsMessageProcessor::handle_command(const string &command) {
  int value;
  if (command == "list-sensors") {
    send_message(to_string(SensorType::TemperatureSensor));
  } else if (sscanf(command.c_str(), "set-delay:%d", &value)) {
    delay_ = value;
  } else if (sscanf(command.c_str(), "set:temperature:%d", &value)) {
    temperature_ = value;
  } else {
    ERROR("Unknown command: " + command);
  }
}

void SensorsMessageProcessor::send_message(const string &msg) {
  send_header(msg.length());
  messenger_->send(msg.c_str(), msg.length());
  finish_message();
}

}  // namespace qemu
}  // namespace anbox
