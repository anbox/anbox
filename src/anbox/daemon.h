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

#ifndef ANBOX_DAEMON_H_
#define ANBOX_DAEMON_H_

#include <string>
#include <vector>

#include "anbox/cli.h"
#include "anbox/do_not_copy_or_move.h"

namespace anbox {
class Daemon : public DoNotCopyOrMove {
 public:
  Daemon();

  int Run(const std::vector<std::string> &arguments);

 private:
  cli::CommandWithSubcommands cmd;
};
}  // namespace anbox

#endif
