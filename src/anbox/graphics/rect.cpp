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

#include "anbox/graphics/rect.h"

#include <algorithm>
#include <string>

namespace anbox {
namespace graphics {
const Rect Rect::Invalid{-1, -1, -1, -1};
const Rect Rect::Empty{0, 0, 0, 0};

void Rect::merge(const Rect &rhs) {
  left_ = std::min(left_, rhs.left());
  top_ = std::min(top_, rhs.top());
  right_ = std::max(right_, rhs.right());
  bottom_ = std::max(bottom_, rhs.bottom());
}

std::ostream &operator<<(std::ostream &out, const Rect &rect) {
  return out << "{" << rect.left() << "," << rect.top() << "," << rect.right()
             << "," << rect.bottom() << "}";
}
}  // namespace graphics
}  // namespace anbox
