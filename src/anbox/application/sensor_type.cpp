
#include "anbox/application/sensor_type.h"

#include <string>
#include <map>

namespace anbox::application {

SensorType SensorTypeHelper::FromString(const std::string& str) {
  static std::map<std::string, SensorType> nameToType;
  if (nameToType.size() == 0) {
    nameToType["acceleration"] = SensorType::AccelerationSensor;
    nameToType["magnetic-field"] = SensorType::MagneticFieldSensor;
    nameToType["orientation"] = SensorType::OrientationSensor;
    nameToType["temperature"] = SensorType::TemperatureSensor;
    nameToType["proximity"] = SensorType::ProximitySensor;
    nameToType["light"] = SensorType::LightSensor;
    nameToType["pressure"] = SensorType::PressureSensor;
    nameToType["humidity"] = SensorType::HumiditySensor;
  }
  if (nameToType.find(str) != nameToType.end())
    return nameToType[str];
  else
    return SensorType::UnknownSensor;
}
}