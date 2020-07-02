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
  }
  double temperature;
};
}  // namespace application
}  // namespace anbox

#endif
