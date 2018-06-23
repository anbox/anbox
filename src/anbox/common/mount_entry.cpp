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

#include "anbox/common/mount_entry.h"
#include "anbox/common/loop_device.h"
#include "anbox/logger.h"

#include <sys/mount.h>

namespace anbox {
namespace common {
std::shared_ptr<MountEntry> MountEntry::create(const boost::filesystem::path &src, const boost::filesystem::path &target,
                                               const std::string &fs_type, unsigned long flags, const std::string &data) {
  auto entry = std::shared_ptr<MountEntry>(new MountEntry(target));
  if (!entry)
    return nullptr;

  const void *mount_data = nullptr;
  if (!data.empty())
    mount_data = reinterpret_cast<const void*>(data.c_str());

  DEBUG("Mounting %s on %s ...", src, target);

  if (::mount(src.c_str(), target.c_str(), !fs_type.empty() ? fs_type.c_str() : nullptr, flags, mount_data) < 0) {
    ERROR("Failed to mount %s: %s", target, strerror(errno));
    return nullptr;
  }

  entry->active_ = true;

  return entry;
}

std::shared_ptr<MountEntry> MountEntry::create(const std::shared_ptr<LoopDevice> &loop, const boost::filesystem::path &target,
                                               const std::string &fs_type, unsigned long flags, const std::string &data) {
  auto entry = create(loop->path(), target, fs_type, flags, data);
  if (!entry)
    return nullptr;

  entry->loop_ = loop;
  return entry;
}

std::shared_ptr<MountEntry> MountEntry::create(const boost::filesystem::path &target) {
  auto entry = std::shared_ptr<MountEntry>(new MountEntry(target));
  if (!entry)
    return nullptr;

  entry->active_ = true;

  return entry;
}

MountEntry::MountEntry(const boost::filesystem::path &target) :
  active_{false}, target_{target} {}

MountEntry::~MountEntry() {
  if (!active_)
    return;

  ::umount(target_.c_str());
}
} // namespace common
} // namespace anbox
