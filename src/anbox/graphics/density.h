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

#ifndef ANBOX_GRAPHICS_DENSITY_H_
#define ANBOX_GRAPHICS_DENSITY_H_

namespace anbox {
namespace graphics {
/**
 * @brief Defines different types of density being used in an Android system.
 * See the
 *        documentation in
 * frameworks/base/core/java/android/util/DisplayMetrics.java
 *        of the Android source tree which defines the different types.
 */
enum class DensityType {
  low = 120,
  medium = 160,
  tv = 213,
  high = 240,
  xhigh = 360,
  xxhigh = 480,
};

DensityType current_density();
int dp_to_pixel(unsigned int dp);
}  // namespace graphics
}  // namespace anbox

#endif
