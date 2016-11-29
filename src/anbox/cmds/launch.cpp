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

#include "anbox/cmds/launch.h"
#include "anbox/dbus/stub/application_manager.h"

#include <core/dbus/asio/executor.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

anbox::cmds::Launch::Launch()
    : CommandWithFlagsAndAction{cli::Name{"launch"}, cli::Usage{"launch"}, cli::Description{"Launch specified application in the Android container"}}
{
    flag(cli::make_flag(cli::Name{"package"}, cli::Description{"Package the application is part of"}, package_));
    flag(cli::make_flag(cli::Name{"activity"}, cli::Description{"Activity of the application to start"}, activity_));
    action([this](const cli::Command::Context&) {
        if (package_.empty() && activity_.empty())
            BOOST_THROW_EXCEPTION(std::runtime_error("Package or activity name not specified"));

        auto bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);
        bus->install_executor(core::dbus::asio::make_executor(bus));
        auto stub = dbus::stub::ApplicationManager::create_for_bus(bus);

        stub->launch(package_, activity_);

        return EXIT_SUCCESS;
    });
}
