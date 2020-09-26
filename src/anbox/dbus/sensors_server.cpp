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

#include "anbox/dbus/sensors_server.h"

#include "anbox/android/intent.h"
#include "anbox/application/sensor_type.h"
#include "anbox/logger.h"
#include "sdbus-c++/Error.h"

sdbus::Struct<double, double, double> SensorsServer::Acceleration() {
  if (impl_->disabled_sensors & anbox::application::SensorType::AccelerationSensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Acceleration sensor is disabled");
  return sdbus::Struct<double, double, double>(impl_->acceleration);
}

void SensorsServer::Acceleration(const sdbus::Struct<double, double, double>& value) {
  if (impl_->disabled_sensors & anbox::application::SensorType::AccelerationSensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Acceleration sensor is disabled");
  impl_->acceleration = value;
}

sdbus::Struct<double, double, double> SensorsServer::MagneticField() {
  if (impl_->disabled_sensors & anbox::application::SensorType::MagneticFieldSensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "MagneticField sensor is disabled");
  return sdbus::Struct<double, double, double>(impl_->magneticField);
}

void SensorsServer::MagneticField(const sdbus::Struct<double, double, double>& value) {
  if (impl_->disabled_sensors & anbox::application::SensorType::MagneticFieldSensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "MagneticField sensor is disabled");
  impl_->magneticField = value;
}

sdbus::Struct<double, double, double> SensorsServer::Orientation() {
  if (impl_->disabled_sensors & anbox::application::SensorType::OrientationSensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Orientation sensor is disabled");
  return sdbus::Struct<double, double, double>(impl_->orientation);
}

void SensorsServer::Orientation(const sdbus::Struct<double, double, double>& value) {
  if (impl_->disabled_sensors & anbox::application::SensorType::OrientationSensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Orientation sensor is disabled");
  impl_->orientation = value;
}

double SensorsServer::Temperature() {
  if (impl_->disabled_sensors & anbox::application::SensorType::TemperatureSensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Temperature sensor is disabled");
  return impl_->temperature;
}

void SensorsServer::Temperature(const double& value) {
  if (impl_->disabled_sensors & anbox::application::SensorType::TemperatureSensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Temperature sensor is disabled");
  impl_->temperature = value;
}

double SensorsServer::Proximity() {
  if (impl_->disabled_sensors & anbox::application::SensorType::ProximitySensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Proximity sensor is disabled");
  return impl_->proximity;
}

void SensorsServer::Proximity(const double& value) {
  if (impl_->disabled_sensors & anbox::application::SensorType::ProximitySensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Proximity sensor is disabled");
  impl_->proximity = value;
}

double SensorsServer::Light() {
  if (impl_->disabled_sensors & anbox::application::SensorType::LightSensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Light sensor is disabled");
  return impl_->light;
}

void SensorsServer::Light(const double& value) {
  if (impl_->disabled_sensors & anbox::application::SensorType::LightSensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Light sensor is disabled");
  impl_->light = value;
}

double SensorsServer::Pressure() {
  if (impl_->disabled_sensors & anbox::application::SensorType::PressureSensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Pressure sensor is disabled");
  return impl_->pressure;
}

void SensorsServer::Pressure(const double& value) {
  if (impl_->disabled_sensors & anbox::application::SensorType::PressureSensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Pressure sensor is disabled");
  impl_->pressure = value;
}

double SensorsServer::Humidity() {
  if (impl_->disabled_sensors & anbox::application::SensorType::HumiditySensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Humidity sensor is disabled");
  return impl_->humidity;
}

void SensorsServer::Humidity(const double& value) {
  if (impl_->disabled_sensors & anbox::application::SensorType::HumiditySensor)
    throw sdbus::Error("org.anbox.SensorDisabled", "Humidity sensor is disabled");
  impl_->humidity = value;
}
