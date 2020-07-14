#include <sdbus-c++/sdbus-c++.h>

#include "ApplicationManagerServerGlue.h"
#include "anbox/application/manager.h"

class ApplicationManagerServer : public sdbus::AdaptorInterfaces<org::anbox::ApplicationManager_adaptor> {
 public:
  ApplicationManagerServer(sdbus::IConnection& connection, std::string objectPath, const std::shared_ptr<anbox::application::Manager>& impl)
      : sdbus::AdaptorInterfaces<org::anbox::ApplicationManager_adaptor>(connection, std::move(objectPath)), impl_(impl) {
    registerAdaptor();
  }

  virtual ~ApplicationManagerServer() {
    unregisterAdaptor();
  }

 protected:
  void Launch(const std::map<std::string, sdbus::Variant>& intentDict, const std::string& arg1) override;
  bool Ready() override;

 private:
  const std::shared_ptr<anbox::application::Manager> impl_;
};
