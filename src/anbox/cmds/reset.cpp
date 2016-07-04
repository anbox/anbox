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

#include "anbox/config.h"
#include "anbox/cmds/reset.h"

namespace fs = boost::filesystem;

anbox::cmds::Reset::Reset()
    : CommandWithFlagsAndAction{cli::Name{"reset"}, cli::Usage{"reset"}, cli::Description{"Reset the container to its default settings"}}
{
    action([this](const cli::Command::Context &) {
        // FIXME(morphis): check if container is currently running and if stop here.

        if (!fs::is_directory(config::data_path()))
            return EXIT_SUCCESS;

        // FIXME(morphis): This currently requires us to run as root as otherwise we can't
        // remove some of the files stored there even if they are created from a user namespace!?
        fs::remove_all(config::data_path());

        return EXIT_SUCCESS;
    });
}
