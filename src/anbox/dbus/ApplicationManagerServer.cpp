
#include "anbox/dbus/ApplicationManagerServer.h"

#include <sstream>

#include "anbox/android/intent.h"
#include "anbox/logger.h"

void ApplicationManagerServer::Launch(const std::map<std::string, sdbus::Variant>& intentDict, const std::string& stack) {
  anbox::android::Intent intent;
  intent.package = intentDict.count("package") ? intentDict.at("package").get<std::string>() : intent.package;
  intent.component = intentDict.count("component") ? intentDict.at("component").get<std::string>() : intent.component;
  intent.action = intentDict.count("action") ? intentDict.at("action").get<std::string>() : intent.action;
  intent.type = intentDict.count("type") ? intentDict.at("type").get<std::string>() : intent.type;
  intent.uri = intentDict.count("uri") ? intentDict.at("uri").get<std::string>() : intent.uri;
  anbox::wm::Stack::Id launch_stack = anbox::wm::Stack::Id::Default;
  if (stack.length() > 0) {
    auto s = std::string(stack);
    std::istringstream i(s);
    i >> launch_stack;
  }

  if (intent.package.length() == 0) {
    throw sdbus::Error("org.anbox.InvalidArgument", "No package specified");
  }

  try {
    impl_->launch(intent, anbox::graphics::Rect::Invalid, launch_stack);
  } catch (std::exception& err) {
    ERROR("Failed to launch application: %s", err.what());
    throw sdbus::Error("org.anbox.InternalError", err.what());
  }
}

bool ApplicationManagerServer::Ready() {
  return impl_->ready();
}
