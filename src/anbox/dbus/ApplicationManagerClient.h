#include <sdbus-c++/sdbus-c++.h>

#include <sstream>

#include "ApplicationManagerClientGlue.h"
#include "anbox/android/intent.h"
#include "anbox/logger.h"
#include "anbox/wm/stack.h"

class ApplicationManagerClient : public sdbus::ProxyInterfaces<org::anbox::ApplicationManager_proxy> {
 public:
  ApplicationManagerClient(sdbus::IConnection& connection, std::string destination, std::string objectPath)
      : sdbus::ProxyInterfaces<org::anbox::ApplicationManager_proxy>(connection, std::move(destination), std::move(objectPath)) {
    registerProxy();
  }

  virtual ~ApplicationManagerClient() {
    unregisterProxy();
  }

  bool TryLaunch(anbox::android::Intent intent, anbox::wm::Stack::Id stack);
};
