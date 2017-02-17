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

#ifndef ANBOX_COMMON_LOOP_DEVICE_ALLOCATOR_H_
#define ANBOX_COMMON_LOOP_DEVICE_ALLOCATOR_H_

#include <memory>

namespace anbox {
namespace common {
class LoopDevice;
class LoopDeviceAllocator {
 public:
  static std::shared_ptr<LoopDevice> new_device();
};
} // namespace common
} // namespace anbox

#endif
