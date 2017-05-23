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

#include "DisplayManager.h"

namespace anbox {
namespace graphics {
namespace emugl {
std::shared_ptr<DisplayInfo> DisplayInfo::get() {
  static auto info = std::make_shared<DisplayInfo>();
  return info;
}

void DisplayInfo::set_resolution(const std::uint32_t &vertical, const std::uint32_t horizontal) {
  vertical_resolution_ = vertical;
  horizontal_resolution_ = horizontal;
}

std::uint32_t DisplayInfo::vertical_resolution() const { return vertical_resolution_; }

std::uint32_t DisplayInfo::horizontal_resolution() const { return horizontal_resolution_; }
} // namespace emugl
} // namespace graphics
} // namespace anbox
