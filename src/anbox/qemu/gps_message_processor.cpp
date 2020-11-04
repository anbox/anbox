/*
 * Copyright (C) 2020 Tomasz Grobelny <tomasz@grobelny.net>
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

#include "anbox/qemu/gps_message_processor.h"

#include <string.h>

#include "anbox/logger.h"

namespace anbox::qemu {
GpsMessageProcessor::GpsMessageProcessor(const std::shared_ptr<network::SocketMessenger> &messenger, const std::shared_ptr<anbox::application::GpsInfoBroker> &gpsInfoBroker) : messenger_(messenger), gps_info_broker_(gpsInfoBroker) {
  connection_ = gps_info_broker_->newNmeaSentence.connect([this](const std::string &sentence) {
    auto dataToSend = sentence + "\n";
    messenger_->send(dataToSend.data(), dataToSend.length());
  });
}

GpsMessageProcessor::~GpsMessageProcessor() {
  gps_info_broker_->newNmeaSentence.disconnect(connection_);
}

bool GpsMessageProcessor::process_data(const std::vector<std::uint8_t> &data) {
  ERROR("Got unexpected GPS data: " + std::to_string(data.size()));
  return true;
}
}