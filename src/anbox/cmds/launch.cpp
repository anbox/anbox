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
    : CommandWithFlagsAndAction{cli::Name{"launch"}, cli::Usage{"launch"}, cli::Description{"Launch an Activity by sending an intent"}}
{
    flag(cli::make_flag(cli::Name{"action"}, cli::Description{"Action of the intent"}, intent_.action));
    flag(cli::make_flag(cli::Name{"type"}, cli::Description{"MIME type for the intent"}, intent_.type));
    flag(cli::make_flag(cli::Name{"uri"}, cli::Description{"URI used as data within the intent"}, intent_.uri));
    flag(cli::make_flag(cli::Name{"package"}, cli::Description{"Package the intent should go to"}, intent_.package));
    flag(cli::make_flag(cli::Name{"component"}, cli::Description{"Component of a package the intent should go"}, intent_.component));

    action([this](const cli::Command::Context&) {
        auto bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);
        bus->install_executor(core::dbus::asio::make_executor(bus));
        auto stub = dbus::stub::ApplicationManager::create_for_bus(bus);

        stub->launch(intent_);

        return EXIT_SUCCESS;
    });
}
