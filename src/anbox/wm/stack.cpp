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

#include "anbox/wm/stack.h"

namespace anbox {
namespace wm {
std::ostream &operator<<(std::ostream &out, const Stack::Id &stack) {
  switch (stack) {
  case anbox::wm::Stack::Id::Default:
    out << "default";
    break;
  case anbox::wm::Stack::Id::Fullscreen:
    out << "fullscreen";
    break;
  case anbox::wm::Stack::Id::Freeform:
    out << "freeform";
    break;
  default:
    break;
  }
  return out;
}

std::istream& operator>>(std::istream& in, Stack::Id &stack) {
  std::string s;
  in >> s;
  if (s == "default")
    stack = anbox::wm::Stack::Id::Default;
  else if (s == "fullscreen")
    stack = anbox::wm::Stack::Id::Fullscreen;
  else if (s == "freeform")
    stack = anbox::wm::Stack::Id::Freeform;
  return in;
}
}  // namespace wm
}  // namespace anbox
