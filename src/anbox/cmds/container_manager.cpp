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

#include "anbox/cmds/container_manager.h"
#include "anbox/container/service.h"
#include "anbox/common/loop_device_allocator.h"
#include "anbox/logger.h"
#include "anbox/runtime.h"
#include "anbox/config.h"

#include "core/posix/signal.h"

#include <sys/mount.h>
#include <linux/loop.h>
#include <fcntl.h>

namespace fs = boost::filesystem;

namespace {
constexpr unsigned int unprivileged_user_id{100000};
}

anbox::cmds::ContainerManager::ContainerManager()
    : CommandWithFlagsAndAction{
          cli::Name{"container-manager"}, cli::Usage{"container-manager"},
          cli::Description{"Start the container manager service"}, true} {

  flag(cli::make_flag(cli::Name{"android-image"},
                      cli::Description{"Path to the Android rootfs image file if not stored in the data path"},
                      android_img_path_));
  flag(cli::make_flag(cli::Name{"data-path"},
                      cli::Description{"Path where the container and its data is stored"},
                      data_path_));
  flag(cli::make_flag(cli::Name{"privileged"},
                      cli::Description{"Run Android container in privileged mode"},
                      privileged_));
  flag(cli::make_flag(cli::Name{"daemon"},
                      cli::Description{"Mark service as being started as systemd daemon"},
                      daemon_));

  action([&](const cli::Command::Context&) {
    try {
      if (!daemon_) {
        WARNING("You are running the container manager manually which is most likely not");
        WARNING("what you want. The container manager is normally started by systemd or");
        WARNING("another init system. If you still want to run the container-manager");
        WARNING("you can get rid of this warning by starting with the --daemon option.");
        WARNING("");
      }

      if (geteuid() != 0) {
        ERROR("You are not running the container-manager as root. Generally you don't");
        ERROR("want to run the container-manager manually unless you're a developer");
        ERROR("as it is started by the init system of your operating system.");
        return EXIT_FAILURE;
      }

      auto trap = core::posix::trap_signals_for_process(
          {core::posix::Signal::sig_term, core::posix::Signal::sig_int});
      trap->signal_raised().connect([trap](const core::posix::Signal& signal) {
        INFO("Signal %i received. Good night.", static_cast<int>(signal));
        trap->stop();
      });

      if (!data_path_.empty())
        SystemConfiguration::instance().set_data_path(data_path_);

      if (!fs::exists(data_path_))
        fs::create_directories(data_path_);

      if (!setup_mounts())
        return EXIT_FAILURE;

      auto rt = Runtime::create();
      auto service = container::Service::create(rt, privileged_);

      rt->start();
      trap->run();
      rt->stop();

      return EXIT_SUCCESS;
    } catch (std::exception &err) {
      ERROR("%s", err.what());
      return EXIT_FAILURE;
    }
  });
}

anbox::cmds::ContainerManager::~ContainerManager() {}

bool anbox::cmds::ContainerManager::setup_mounts() {
  fs::path android_img_path = android_img_path_;
  if (android_img_path.empty())
    android_img_path = SystemConfiguration::instance().data_dir() / "android.img";

  if (!fs::exists(android_img_path)) {
    ERROR("Android image does not exist at path %s", android_img_path);
    return false;
  }

  const auto android_rootfs_dir = SystemConfiguration::instance().rootfs_dir();
  if (utils::is_mounted(android_rootfs_dir)) {
    ERROR("Androd rootfs is already mounted!?");
    return false;
  }

  if (!fs::exists(android_rootfs_dir))
    fs::create_directory(android_rootfs_dir);

  std::shared_ptr<common::LoopDevice> loop_device;

  try {
    loop_device = common::LoopDeviceAllocator::new_device();
  } catch (const std::exception& e) {
    ERROR("Could not create loopback device: %s", e.what());
    return false;
  } catch (...) {
    ERROR("Could not create loopback device");
    return false;
  }

  if (!loop_device->attach_file(android_img_path)) {
    ERROR("Failed to attach Android rootfs image to loopback device");
    return false;
  }

  auto m = common::MountEntry::create(loop_device, android_rootfs_dir, "squashfs", MS_MGC_VAL | MS_RDONLY | MS_PRIVATE);
  if (!m) {
    ERROR("Failed to mount Android rootfs");
    return false;
  }
  mounts_.push_back(m);

  for (const auto &dir_name : std::vector<std::string>{"cache", "data"}) {
    auto target_dir_path = fs::path(android_rootfs_dir) / dir_name;
    auto src_dir_path = SystemConfiguration::instance().data_dir() / dir_name;

    if (!fs::exists(src_dir_path)) {
      if (!fs::create_directory(src_dir_path)) {
        ERROR("Failed to create Android %s directory", dir_name);
        mounts_.clear();
        return false;
      }
      if (::chown(src_dir_path.c_str(), unprivileged_user_id, unprivileged_user_id) != 0) {
        ERROR("Failed to allow access for unprivileged user on %s directory of the rootfs", dir_name);
        mounts_.clear();
        return false;
      }
    }

    auto m = common::MountEntry::create(src_dir_path, target_dir_path, "", MS_MGC_VAL | MS_BIND | MS_PRIVATE);
    if (!m) {
      ERROR("Failed to mount Android %s directory", dir_name);
      mounts_.clear();
      return false;
    }
    mounts_.push_back(m);
  }

  // Unmounting needs to happen in reverse order
  std::reverse(mounts_.begin(), mounts_.end());

  return true;
}
