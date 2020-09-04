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
#include "anbox/logger.h"

double SensorsServer::Temperature() {
  return impl_->temperature;
}

void SensorsServer::Temperature(const double& value) {
  impl_->temperature = value;
}

double SensorsServer::Proximity() {
  return impl_->proximity;
}

void SensorsServer::Proximity(const double& value) {
  impl_->proximity = value;
}

double SensorsServer::Light() {
  return impl_->light;
}

void SensorsServer::Light(const double& value) {
  impl_->light = value;
}

double SensorsServer::Pressure() {
  return impl_->pressure;
}

void SensorsServer::Pressure(const double& value) {
  impl_->pressure = value;
}

double SensorsServer::Humidity() {
  return impl_->humidity;
}

void SensorsServer::Humidity(const double& value) {
  impl_->humidity = value;
}
