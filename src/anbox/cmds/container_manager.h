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

#ifndef ANBOX_CMDS_CONTAINER_MANAGER_H_
#define ANBOX_CMDS_CONTAINER_MANAGER_H_

#include <functional>
#include <iostream>
#include <memory>

#include "anbox/cli.h"

#include "anbox/common/loop_device.h"
#include "anbox/common/mount_entry.h"

namespace anbox {
namespace cmds {
class ContainerManager : public cli::CommandWithFlagsAndAction {
 public:
  ContainerManager();
  ~ContainerManager();

 private:
  bool setup_mounts();
  bool setup_rootfs_overlay();

  std::string android_img_path_;
  std::string data_path_;
  std::shared_ptr<common::LoopDevice> android_img_loop_dev_;
  std::vector<std::shared_ptr<common::MountEntry>> mounts_;
  bool privileged_ = false;
  bool daemon_ = false;
  bool enable_rootfs_overlay_ = false;
  bool enable_squashfuse_ = false;
  std::string container_network_address_;
  std::string container_network_gateway_;
  std::string container_network_dns_servers_;
};
}  // namespace cmds
}  // namespace anbox

#endif
