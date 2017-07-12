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

#ifndef ANBOX_COMMON_MOUNT_ENTRY_H_
#define ANBOX_COMMON_MOUNT_ENTRY_H_

#include <boost/filesystem/path.hpp>

namespace anbox {
namespace common {
class LoopDevice;
class MountEntry {
 public:
  static std::shared_ptr<MountEntry> create(const boost::filesystem::path &src, const boost::filesystem::path &target,
                                            const std::string &fs_type = "", unsigned long flags = 0, const std::string &data = "");

  static std::shared_ptr<MountEntry> create(const std::shared_ptr<LoopDevice> &loop, const boost::filesystem::path &target,
                                            const std::string &fs_type = "", unsigned long flags = 0, const std::string &data = "");

  static std::shared_ptr<MountEntry> create(const boost::filesystem::path &target);

  ~MountEntry();

 private:
  MountEntry(const boost::filesystem::path &target);

  bool active_;
  std::shared_ptr<LoopDevice> loop_;
  boost::filesystem::path target_;
};
} // namespace common
} // namespace anbox

#endif
