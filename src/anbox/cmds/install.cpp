/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
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

#include "anbox/cmds/install.h"
#include "anbox/dbus/stub/application_manager.h"

#include <core/dbus/asio/executor.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

anbox::cmds::Install::Install()
    : CommandWithFlagsAndAction{
          cli::Name{"install"}, cli::Usage{"install"},
          cli::Description{
              "Install specified application in the Android container"}} {
  flag(cli::make_flag(cli::Name{"apk"},
                      cli::Description{"Path to APK to install"}, apk_));
  action([this](const cli::Command::Context&) {
    if (apk_.length() == 0)
      BOOST_THROW_EXCEPTION(std::runtime_error("No APK to install specified"));

    if (!fs::is_regular_file(apk_))
      BOOST_THROW_EXCEPTION(
          std::runtime_error("Specified APK file does not exist"));

    auto bus =
        std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);
    bus->install_executor(core::dbus::asio::make_executor(bus));
    auto stub = dbus::stub::ApplicationManager::create_for_bus(bus);

    stub->install(apk_);

    return EXIT_SUCCESS;
  });
}
