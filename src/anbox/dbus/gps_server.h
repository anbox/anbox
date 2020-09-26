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

#ifndef ANBOX_DBUS_GPS_SERVER_H_
#define ANBOX_DBUS_GPS_SERVER_H_

#include <sdbus-c++/sdbus-c++.h>

#include "gps_server_glue.h"
#include "anbox/application/gps_info_broker.h"

class GpsServer : public sdbus::AdaptorInterfaces<org::anbox::Gps_adaptor> {
 public:
  GpsServer(sdbus::IConnection& connection, std::string objectPath, const std::shared_ptr<anbox::application::GpsInfoBroker>& gpsInfoBroker)
      : sdbus::AdaptorInterfaces<org::anbox::Gps_adaptor>(connection, std::move(objectPath)), gps_info_broker_(gpsInfoBroker) {
    registerAdaptor();
  }

  virtual ~GpsServer() {
    unregisterAdaptor();
  }

  void PushSentence(const std::string& sentence) override;

 private:
  const std::shared_ptr<anbox::application::GpsInfoBroker> gps_info_broker_;
};

#endif
