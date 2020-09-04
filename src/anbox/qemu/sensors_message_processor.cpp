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

#include <fmt/core.h>

#include <boost/algorithm/string/predicate.hpp>
#include <chrono>
#include <iostream>
#include <thread>

#include "anbox/application/sensor_type.h"
#include "anbox/logger.h"

using namespace std;
using namespace anbox::application;

namespace anbox {
namespace qemu {
SensorsMessageProcessor::SensorsMessageProcessor(
    shared_ptr<network::SocketMessenger> messenger, shared_ptr<application::SensorsState> sensorsState)
    : QemudMessageProcessor(messenger), sensors_state_(sensorsState) {
  thread_ = std::thread([this]() {
    for (;;) {
      bool atLeastOneEnabled = false;
      if (temperature_.load()) {
        send_message(fmt::format("temperature:{0:.1f}", sensors_state_->temperature));
        atLeastOneEnabled = true;
      }
      if (proximity_.load()) {
        send_message(fmt::format("proximity:{0:.2f}", sensors_state_->proximity));
        atLeastOneEnabled = true;
      }
      if (light_.load()) {
        send_message(fmt::format("light:{0:.2f}", sensors_state_->light));
        atLeastOneEnabled = true;
      }
      if (pressure_.load()) {
        send_message(fmt::format("pressure:{0:.2f}", sensors_state_->pressure));
        atLeastOneEnabled = true;
      }
      if (humidity_.load()) {
        send_message(fmt::format("humidity:{0:.2f}", sensors_state_->humidity));
        atLeastOneEnabled = true;
      }
      if (atLeastOneEnabled) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        send_message(fmt::format("sync:{0:d}", tv.tv_sec * 1000000LL + tv.tv_usec));
      }
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
    uint8_t enabledSensors = 0;
    enabledSensors |= SensorType::TemperatureSensor;
    enabledSensors |= SensorType::ProximitySensor;
    enabledSensors |= SensorType::LightSensor;
    enabledSensors |= SensorType::PressureSensor;
    enabledSensors |= SensorType::HumiditySensor;
    enabledSensors &= ~sensors_state_->disabled_sensors;
    send_message(to_string(enabledSensors));
  } else if (sscanf(command.c_str(), "set-delay:%d", &value)) {
    delay_ = value;
  } else if (sscanf(command.c_str(), "set:temperature:%d", &value)) {
    temperature_ = value;
  } else if (sscanf(command.c_str(), "set:proximity:%d", &value)) {
    proximity_ = value;
  } else if (sscanf(command.c_str(), "set:light:%d", &value)) {
    light_ = value;
  } else if (sscanf(command.c_str(), "set:pressure:%d", &value)) {
    pressure_ = value;
  } else if (sscanf(command.c_str(), "set:humidity:%d", &value)) {
    humidity_ = value;
  } else {
    ERROR("Unknown command: " + command);
  }
}

void SensorsMessageProcessor::send_message(const string &msg) {
  send_header(msg.length());
  messenger_->send(msg.c_str(), msg.length());
}

}  // namespace qemu
}  // namespace anbox
