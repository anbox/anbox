#ifndef ANBOX_APPLICATION_SENSORS_STATE_H_
#define ANBOX_APPLICATION_SENSORS_STATE_H_

#include <cstdint>
#include <tuple>

#include "anbox/do_not_copy_or_move.h"
#include "anbox/logger.h"

namespace anbox::application {
struct SensorsState : public DoNotCopyOrMove {
  SensorsState() {
    disabled_sensors = 0;
    acceleration = {0, 0, 9.80665};
    magneticField = {25, 25, 5};
    orientation = {0, 0, 0};
    temperature = 25;
    proximity = 5;
    light = 1240;
    pressure = 1013.25;
    humidity = 45.1;
  }
  int disabled_sensors;
  std::tuple<double, double, double> acceleration;
  std::tuple<double, double, double> magneticField;
  std::tuple<double, double, double> orientation;
  double temperature;
  double proximity;
  double light;
  double pressure;
  double humidity;
};
}
#endif
