/*
 * Copyright (C) 2018 Canonical Ltd.
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

#include "anbox/common/binder_device.h"
#include "anbox/common/binderfs.h"
#include "anbox/defer_action.h"
#include "anbox/logger.h"
#include "anbox/utils.h"

#include <system_error>

#include <cerrno>
#include <fcntl.h>
#include <linux/loop.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

namespace anbox {
namespace common {
std::unique_ptr<BinderDevice> BinderDevice::create(const std::string& path) {
  return std::unique_ptr<BinderDevice>(new BinderDevice(path));
}

BinderDevice::BinderDevice(const std::string& path) :
    path_{path} {
  if (::chmod(path.c_str(), 0666) < 0)
    ERROR("Failed to change permissions of binder node %s: %s", path, std::strerror(errno));
}

BinderDevice::~BinderDevice() {
  if (::unlink(path_.c_str()) < 0)
    ERROR("Failed to remove binder node %s: %s", path_, std::strerror(errno));
}
} // namespace common
} // namespace anbox
