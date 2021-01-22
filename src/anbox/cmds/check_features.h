/*
 * Copyright (C) 2018 Simon Fels <morphis@gravedo.de>
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

#pragma once

#include <functional>
#include <iostream>
#include <memory>

#include "anbox/cli.h"

namespace anbox::cmds {
class CheckFeatures : public cli::CommandWithFlagsAndAction {
 public:
  CheckFeatures();

 private:
  bool sanity_check_for_features();
};
}
