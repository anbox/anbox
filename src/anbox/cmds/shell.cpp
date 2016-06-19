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

#include <boost/filesystem.hpp>

#include "core/posix/signal.h"

#include "anbox/logger.h"
#include "anbox/config.h"
#include "anbox/container_connector.h"
#include "anbox/cmds/shell.h"

namespace fs = boost::filesystem;

anbox::cmds::Shell::Shell()
    : CommandWithFlagsAndAction{cli::Name{"shell"}, cli::Usage{"shell"}, cli::Description{"Open a shell within the Anbox container"}},
      pid_(-1)
{
    flag(cli::make_flag(cli::Name{"pid"}, cli::Description{"PID of container to attach to"}, pid_));
    action([this](const cli::Command::Context &) {
        ContainerConnector connector(pid_);
        return connector.run("/system/bin/sh");
    });
}
