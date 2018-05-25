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

#include "anbox/graphics/density.h"

namespace anbox {
namespace graphics {
DensityType current_density() {
  return DensityType::medium;
}

int dp_to_pixel(unsigned int dp) {
  return dp * static_cast<unsigned int>(current_density()) / static_cast<unsigned int>(DensityType::medium);
}
}  // namespace graphics
}  // namespace anbox
