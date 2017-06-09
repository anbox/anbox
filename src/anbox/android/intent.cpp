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
bool Intent::valid() const {
  // At the moment we only support component+package for intents
  // (see android/service/android_api_skeleton.cpp for more details)
  return !(component.empty() && package.empty());
}

std::ostream &operator<<(std::ostream &out, const Intent &intent) {
  out << "[";
  if (!intent.action.empty())
    out << " " << "action=" << intent.action << " ";
  if (!intent.uri.empty())
    out << " " << "uri=" << intent.uri << " ";
  if (!intent.type.empty())
    out << " " << "type=" << intent.type  << " ";
  if (intent.flags > 0)
    out << " " << "flags=" << intent.flags  << " ";
  if (!intent.package.empty())
    out << " " << "package=" << intent.package << " ";
  if (!intent.component.empty())
    out << "component=" << intent.component << " ";
  if (intent.categories.size() > 0) {
    out << "categories=[ ";
    for (const auto &category : intent.categories) out << category << " ";
    out << "] ";
  }
  out << "]";
  return out;
}
}  // namespace android
}  // namespace anbox
