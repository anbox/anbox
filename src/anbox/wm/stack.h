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

#ifndef ANBOX_WM_STACK_H_
#define ANBOX_WM_STACK_H_

#include <ostream>

namespace anbox {
namespace wm {
class Stack {
 public:
  enum class Id {
    Invalid = -1,
    Default = 0,
    Fullscreen = 1,
    Freeform = 2,
  };

  Stack() = delete;
  Stack(const Stack&) = delete;
};

std::ostream& operator<<(std::ostream &out, Stack::Id const &stack);
std::istream& operator>>(std::istream &in, Stack::Id &stack);
}  // namespace wm
}  // namespace anbox



#endif
