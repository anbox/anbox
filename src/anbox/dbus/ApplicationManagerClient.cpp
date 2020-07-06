#include "ApplicationManagerClient.h"

bool ApplicationManagerClient::TryLaunch(anbox::android::Intent intent, anbox::wm::Stack::Id stack) {
  try {
    DEBUG("Sending launch intent %s to Android ..", intent);
    std::map<std::string, sdbus::Variant> intentDict;
    std::ostringstream launch_stack;
    launch_stack << stack;
    if (intent.action.length())
      intentDict["action"] = sdbus::Variant(intent.action);
    if (intent.component.length())
      intentDict["component"] = sdbus::Variant(intent.component);
    if (intent.package.length())
      intentDict["package"] = sdbus::Variant(intent.package);
    if (intent.type.length())
      intentDict["type"] = sdbus::Variant(intent.type);
    if (intent.uri.length())
      intentDict["uri"] = sdbus::Variant(intent.uri);
    this->Launch(intentDict, launch_stack.str());
  } catch (const std::exception &err) {
    ERROR("Failed to launch activity: %s", err.what());
    return false;
  } catch (...) {
    ERROR("Failed to launch activity");
    return false;
  }

  return true;
}
