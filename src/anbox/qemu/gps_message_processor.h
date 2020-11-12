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

#ifndef ANBOX_QEMU_GPS_MESSAGE_PROCESSOR_H_
#define ANBOX_QEMU_GPS_MESSAGE_PROCESSOR_H_

#include "anbox/application/gps_info_broker.h"
#include "anbox/network/message_processor.h"
#include "anbox/network/socket_messenger.h"

namespace anbox::qemu {
class GpsMessageProcessor : public network::MessageProcessor {
 public:
  GpsMessageProcessor(const std::shared_ptr<network::SocketMessenger> &messenger, const std::shared_ptr<anbox::application::GpsInfoBroker> &gpsInfoBroker);
  ~GpsMessageProcessor();

  bool process_data(const std::vector<std::uint8_t> &data) override;

 private:
  std::shared_ptr<network::SocketMessenger> messenger_;
  std::shared_ptr<anbox::application::GpsInfoBroker> gps_info_broker_;
  boost::signals2::connection connection_;
};
}
#endif
