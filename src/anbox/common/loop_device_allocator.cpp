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

#include "anbox/common/loop_device_allocator.h"
#include "anbox/common/loop_device.h"
#include "anbox/defer_action.h"
#include "anbox/utils.h"

#include <boost/filesystem.hpp>

#include <linux/loop.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <system_error>

namespace fs = boost::filesystem;

namespace {
const constexpr char *loop_control_path{"/dev/loop-control"};
const constexpr char *base_loop_path{"/dev/loop"};
}

namespace anbox {
namespace common {
std::shared_ptr<LoopDevice> LoopDeviceAllocator::new_device() {
  const auto ctl_fd = ::open(loop_control_path, O_RDWR);
  if (ctl_fd < 0)
    throw std::system_error{errno, std::system_category()};

  DeferAction close_ctl_fd{[&]() { ::close(ctl_fd); }};

  const auto device_nr = ::ioctl(ctl_fd, LOOP_CTL_GET_FREE);
  if (device_nr < 0)
    throw std::system_error{errno, std::system_category()};

  return LoopDevice::create(utils::string_format("%s%d", base_loop_path, device_nr));
}
} // namespace common
} // namespace anbox
