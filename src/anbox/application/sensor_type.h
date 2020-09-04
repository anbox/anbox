#ifndef ANBOX_APPLICATION_SENSOR_TYPE_H_
#define ANBOX_APPLICATION_SENSOR_TYPE_H_

#include <cstdint>

#include "anbox/do_not_copy_or_move.h"
#include "anbox/logger.h"

namespace anbox {
namespace application {
enum SensorType {
  TemperatureSensor = (1 << 3),
  ProximitySensor = (1 << 4),
  LightSensor = (1 << 5),
  PressureSensor = (1 << 6),
  HumiditySensor = (1 << 7),
};
}  // namespace application
}  // namespace anbox

#endif
