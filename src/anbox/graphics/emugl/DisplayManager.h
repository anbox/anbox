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

#ifndef ANBOX_GRAPHICS_EMUGL_DISPLAY_INFO_H_
#define ANBOX_GRAPHICS_EMUGL_DISPLAY_INFO_H_

#include <cstdint>
#include <memory>

namespace anbox {
namespace graphics {
namespace emugl {
class DisplayInfo {
 public:
  DisplayInfo() = default;

  static std::shared_ptr<DisplayInfo> get();

  void set_resolution(const std::uint32_t &vertical, const std::uint32_t horizontal);

  std::uint32_t vertical_resolution() const;
  std::uint32_t horizontal_resolution() const;

 private:
  std::uint32_t vertical_resolution_ = 1280;
  std::uint32_t horizontal_resolution_ = 720;
};
} // namespace emugl
} // namespace graphics
} // namespace anbox

#endif
