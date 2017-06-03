/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
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

#include "anbox/build/version.h"
#include "anbox/utils.h"

#include <cstring>

namespace anbox {
namespace build {
static std::string init_version_string() {
  std::string v;
  if (!version_suffix || std::strlen(version_suffix) == 0)
    v = utils::string_format("%s", version);
  else
    v = utils::string_format("%s-%s", version, version_suffix);
  return v;
}

std::string print_version() {
  static const std::string v{init_version_string()};
  return v;
}
}  // namespace build
}  // namespace anbox
