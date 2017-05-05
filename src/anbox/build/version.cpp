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

namespace anbox {
namespace build {
std::string print_version() {
  static std::string v;
  if (v.empty()) {
    if (version_suffix.empty())
      v = utils::string_format("%d", version_major);
    else
      v = utils::string_format("%d-%s", version_major, version_suffix);
  }
  return v;
}
}  // namespace build
}  // namespace anbox
