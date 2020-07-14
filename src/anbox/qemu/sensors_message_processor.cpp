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
#include <chrono>
#include <thread>

#include "anbox/logger.h"

using namespace std;

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
  if (!(list_sensors(command) || handle_set(command) || handle_set_delay(command)))
    ERROR("Unknown command: " + command);
}

bool SensorsMessageProcessor::handle_set_delay(const string &command) {
  if (!boost::starts_with(command, "set-delay:"))
    return false;
  delay = boost::lexical_cast<int>(command.substr("set-delay:"s.length()));
  return true;
}

bool SensorsMessageProcessor::handle_set(const string &command) {
  if (!boost::starts_with(command, "set:temperature:"))
    return false;
  temperature = static_cast<bool>(boost::lexical_cast<int>(command.substr("set:temperature:"s.length())));
  return true;
}

bool SensorsMessageProcessor::list_sensors(const string &command) {
  if (command != "list-sensors")
    return false;
  int mask = 8;
  send_message(to_string(mask));
  return true;
}

void SensorsMessageProcessor::send_message(const string &msg) {
  send_header(msg.length());
  messenger_->send(msg.c_str(), msg.length());
  finish_message();
}

}  // namespace qemu
}  // namespace anbox
