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

#ifndef ANBOX_DBUS_SENSORS_SERVER_H_
#define ANBOX_DBUS_SENSORS_SERVER_H_

#include <sdbus-c++/sdbus-c++.h>

#include "anbox/application/sensors_state.h"
#include "sensors_server_glue.h"

class SensorsServer : public sdbus::AdaptorInterfaces<org::anbox::Sensors_adaptor> {
 public:
  SensorsServer(sdbus::IConnection& connection, std::string objectPath, const std::shared_ptr<anbox::application::SensorsState>& impl)
      : sdbus::AdaptorInterfaces<org::anbox::Sensors_adaptor>(connection, std::move(objectPath)), impl_(impl) {
    registerAdaptor();
  }

  virtual ~SensorsServer() {
    unregisterAdaptor();
  }

  sdbus::Struct<double, double, double> Acceleration() override;
  void Acceleration(const sdbus::Struct<double, double, double>& value) override;
  sdbus::Struct<double, double, double> MagneticField() override;
  void MagneticField(const sdbus::Struct<double, double, double>& value) override;
  sdbus::Struct<double, double, double> Orientation() override;
  void Orientation(const sdbus::Struct<double, double, double>& value) override;
  double Temperature() override;
  void Temperature(const double& value) override;
  double Proximity() override;
  void Proximity(const double& value) override;
  double Light() override;
  void Light(const double& value) override;
  double Pressure() override;
  void Pressure(const double& value) override;
  double Humidity() override;
  void Humidity(const double& value) override;

 private:
  const std::shared_ptr<anbox::application::SensorsState> impl_;
};

#endif
