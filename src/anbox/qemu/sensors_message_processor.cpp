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

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <chrono>
#include <iostream>
#include <thread>

#include "anbox/application/sensor_type.h"
#include "anbox/logger.h"

using namespace std;
using namespace anbox::application;

namespace std {
std::ostream& operator<<(std::ostream& os, std::tuple<double, double, double> t) {
  os << std::get<0>(t) << ":" << std::get<1>(t) << ":" << std::get<2>(t);
  return os;
}
}  // namespace std


namespace anbox::qemu {
SensorsMessageProcessor::SensorsMessageProcessor(
    shared_ptr<network::SocketMessenger> messenger, shared_ptr<application::SensorsState> sensorsState)
    : QemudMessageProcessor(messenger), sensors_state_(sensorsState) {
  enabledSensors_ = 0;
  thread_ = std::thread([this]() {
    for (;;) {
      auto enabledSensors = enabledSensors_.load();
      if (enabledSensors & SensorType::AccelerationSensor)
        send_message(utils::string_format("acceleration:%1%", sensors_state_->acceleration));
      if (enabledSensors & SensorType::MagneticFieldSensor)
        send_message(utils::string_format("magnetic:%1%", sensors_state_->magneticField));
      if (enabledSensors & SensorType::OrientationSensor)
        send_message(utils::string_format("orientation:%1%", sensors_state_->orientation));
      if (enabledSensors & SensorType::TemperatureSensor)
        send_message(utils::string_format("temperature:%1%", sensors_state_->temperature));
      if (enabledSensors & SensorType::ProximitySensor)
        send_message(utils::string_format("proximity:%1%", sensors_state_->proximity));
      if (enabledSensors & SensorType::LightSensor)
        send_message(utils::string_format("light:%1%", sensors_state_->light));
      if (enabledSensors & SensorType::PressureSensor)
        send_message(utils::string_format("pressure:%1%", sensors_state_->pressure));
      if (enabledSensors & SensorType::HumiditySensor)
        send_message(utils::string_format("humidity:%1%", sensors_state_->humidity));
      if (enabledSensors) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        send_message(utils::string_format("sync:%d", tv.tv_sec * 1000000LL + tv.tv_usec));
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

void SensorsMessageProcessor::handle_command(const string& command) {
  int value;
  std::vector<string> parts;
  boost::split(parts, command, boost::is_any_of(":"));
  if (command == "list-sensors") {
    uint32_t enabledSensors = 0;
    enabledSensors |= SensorType::AccelerationSensor;
    enabledSensors |= SensorType::MagneticFieldSensor;
    enabledSensors |= SensorType::OrientationSensor;
    enabledSensors |= SensorType::TemperatureSensor;
    enabledSensors |= SensorType::ProximitySensor;
    enabledSensors |= SensorType::LightSensor;
    enabledSensors |= SensorType::PressureSensor;
    enabledSensors |= SensorType::HumiditySensor;
    enabledSensors &= ~sensors_state_->disabled_sensors;
    send_message(to_string(enabledSensors));
  } else if (sscanf(command.c_str(), "set-delay:%d", &value)) {
    delay_ = value;
  } else if (parts.size() == 3 && parts[0] == "set") {
    auto st = SensorTypeHelper::FromString(parts[1]);
    if (parts[2] == "1") {
      enabledSensors_ |= st;
    } else {
      enabledSensors_ &= ~st;
    }
  } else {
    ERROR("Unknown command: " + command);
  }
}  // namespace qemu

void SensorsMessageProcessor::send_message(const string& msg) {
  send_header(msg.length());
  messenger_->send(msg.c_str(), msg.length());
}

}
