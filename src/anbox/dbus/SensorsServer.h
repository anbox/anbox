#include <sdbus-c++/sdbus-c++.h>

#include "SensorsServerGlue.h"
#include "anbox/application/sensors_state.h"

class SensorsServer : public sdbus::AdaptorInterfaces<org::anbox::Sensors_adaptor> {
 public:
  SensorsServer(sdbus::IConnection& connection, std::string objectPath, const std::shared_ptr<anbox::application::SensorsState>& impl)
      : sdbus::AdaptorInterfaces<org::anbox::Sensors_adaptor>(connection, std::move(objectPath)), impl_(impl) {
    registerAdaptor();
  }

  virtual ~SensorsServer() {
    unregisterAdaptor();
  }

  double Temperature() override;
  void Temperature(const double& value) override;

 private:
  const std::shared_ptr<anbox::application::SensorsState> impl_;
};
