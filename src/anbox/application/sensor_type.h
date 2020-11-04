#ifndef ANBOX_APPLICATION_SENSOR_TYPE_H_
#define ANBOX_APPLICATION_SENSOR_TYPE_H_

#include <string>

namespace anbox::application {
enum SensorType {
  UnknownSensor = 0,
  AccelerationSensor = (1 << 0),
  MagneticFieldSensor = (1 << 1),
  OrientationSensor = (1 << 2),
  TemperatureSensor = (1 << 3),
  ProximitySensor = (1 << 4),
  LightSensor = (1 << 5),
  PressureSensor = (1 << 6),
  HumiditySensor = (1 << 7),
};
class SensorTypeHelper {
 public:
  static SensorType FromString(const std::string& str);
};
}
#endif
