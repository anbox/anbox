#include "anbox/dbus/SensorsServer.h"

#include "anbox/android/intent.h"
#include "anbox/logger.h"

double SensorsServer::Temperature() {
  return impl_->temperature;
}

void SensorsServer::Temperature(const double& value) {
  impl_->temperature = value;
}
