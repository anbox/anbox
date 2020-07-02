/*
 * Copyright (C) 2020 Tomasz Grobelny <tomasz@grobelny.net>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "anbox/dbus/application_manager_server.h"

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
    if (!impl_->ready())
      throw std::runtime_error("Anbox not yet ready to launch applications");

    DEBUG("Launching %s", intent);
    impl_->launch(intent, anbox::graphics::Rect::Invalid, launch_stack);
  } catch (std::exception& err) {
    ERROR("Failed to launch application: %s", err.what());
    throw sdbus::Error("org.anbox.InternalError", err.what());
  }
}

bool ApplicationManagerServer::Ready() {
  return impl_->ready();
}
