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

#include "anbox/android/intent.h"

#include <ostream>

namespace anbox {
namespace android {
std::ostream &operator<<(std::ostream &out, const Intent &intent) {
  out << "["
      << "action=" << intent.action << " "
      << "uri=" << intent.uri << " "
      << "type=" << intent.type << " "
      << "flags=" << intent.flags << " "
      << "package=" << intent.package << " "
      << "component=" << intent.component << " "
      << "categories=[ ";
  for (const auto &category : intent.categories) out << category << " ";
  out << "]]";
  return out;
}
}  // namespace android
}  // namespace anbox
