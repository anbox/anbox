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
#include <boost/lexical_cast.hpp>
#include <boost/spirit/home/x3.hpp>
#include <chrono>
#include <iostream>
#include <thread>

#include "anbox/logger.h"

using namespace std;
namespace x3 = boost::spirit::x3;

namespace anbox {
namespace qemu {
SensorsMessageProcessor::SensorsMessageProcessor(
    shared_ptr<network::SocketMessenger> messenger, shared_ptr<application::SensorsState> sensorsState)
    : QemudMessageProcessor(messenger), sensorsState_(sensorsState) {
  thread = std::thread([this]() {
    for (;;) {
      if (temperature.load())
        send_message("temperature:" + to_string(sensorsState_->temperature));
      if (!run_thread.load())
        break;
      this_thread::sleep_for(delay.load() * 1ms);
    }
  });
}

SensorsMessageProcessor::~SensorsMessageProcessor() {
  run_thread = false;
  thread.join();
}

void SensorsMessageProcessor::handle_command(const string &command) {
  auto list_sensors = [this](auto &) { send_message(to_string(SensorType::TemperatureSensor)); };
  auto set_delay = [this](auto &ctx) { delay = _attr(ctx); };
  auto set_temperature = [this](auto &ctx) { temperature = _attr(ctx); };
  auto set_command_parser = ("set:temperature:" >> x3::int_[set_temperature]);
  auto general_command_parser = x3::lit("list-sensors")[list_sensors] | ("set-delay:" >> x3::int_[set_delay]);
  if (!parse(command.begin(), command.end(), general_command_parser | set_command_parser))
    ERROR("Unknown command: " + command);
}

void SensorsMessageProcessor::send_message(const string &msg) {
  send_header(msg.length());
  messenger_->send(msg.c_str(), msg.length());
  finish_message();
}

}  // namespace qemu
}  // namespace anbox
