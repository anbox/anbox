/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
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
 */

#ifndef ANBOX_CMDS_SYSTEM_INFO_H_
#define ANBOX_CMDS_SYSTEM_INFO_H_

#include <functional>
#include <iostream>
#include <memory>

#include "anbox/cli.h"

namespace anbox {
namespace cmds {
class SystemInfo : public cli::CommandWithFlagsAndAction {
 public:
  SystemInfo();
};
}  // namespace cmds
}  // namespace anbox

#endif
