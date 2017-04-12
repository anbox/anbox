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

#include "anbox/common/loop_device.h"
#include "anbox/defer_action.h"

#include <system_error>

#include <linux/loop.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

namespace anbox {
namespace common {
std::shared_ptr<LoopDevice> LoopDevice::create(const boost::filesystem::path &path) {
  const auto fd = ::open(path.c_str(), O_RDWR);
  if (fd < 0)
    throw std::system_error{errno, std::system_category()};

  return std::shared_ptr<LoopDevice>(new LoopDevice(Fd{fd}, path));
}

LoopDevice::LoopDevice(Fd fd, const boost::filesystem::path &path) :
  fd_{fd}, path_{path} {}

LoopDevice::~LoopDevice() {
  if (fd_ < 0)
    return;

  ::ioctl(fd_, LOOP_CLR_FD);
  ::close(fd_);
}

bool LoopDevice::attach_file(const boost::filesystem::path &file_path) {
  if (fd_ < 0)
    return false;

  int file_fd = ::open(file_path.c_str(), O_RDONLY);
  if (file_fd < 0)
    return false;

  DeferAction close_file_fd{[&]() { ::close(file_fd); }};

  if (::ioctl(fd_, LOOP_SET_FD, file_fd) < 0)
    return false;

  return true;
}
} // namespace common
} // namespace anbox
