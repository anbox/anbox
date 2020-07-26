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

#include <sdbus-c++/sdbus-c++.h>

#include <sstream>

#include "application_manager_client_glue.h"
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
