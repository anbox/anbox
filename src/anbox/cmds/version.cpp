/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 *
 */

#include "anbox/cmds/version.h"
#include "anbox/build/config.h"
#include "anbox/utils.h"

anbox::cmds::Version::Version()
    : CommandWithFlagsAndAction{
          cli::Name{"version"}, cli::Usage{"version"},
          cli::Description{"Print the version of the daemon"}} {
  action([](const cli::Command::Context& ctxt) {
    ctxt.cout << "anbox " << build::version << std::endl;
    return 0;
  });
}
