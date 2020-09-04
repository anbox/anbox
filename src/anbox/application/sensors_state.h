#ifndef ANBOX_APPLICATION_SENSORS_STATE_H_
#define ANBOX_APPLICATION_SENSORS_STATE_H_

#include <cstdint>

#include "anbox/do_not_copy_or_move.h"
#include "anbox/logger.h"

namespace anbox {
namespace application {
struct SensorsState : public DoNotCopyOrMove {
  SensorsState() {
    temperature = 25;
    proximity = 5;
    light = 1240;
    pressure = 1013.25;
    humidity = 45.1;
  }
  double temperature;
  double proximity;
  double light;
  double pressure;
  double humidity;
};
}  // namespace application
}  // namespace anbox

#endif
