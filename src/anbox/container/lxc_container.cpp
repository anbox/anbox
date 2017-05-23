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

#include "anbox/android/ip_config_builder.h"
#include "anbox/container/lxc_container.h"
#include "anbox/config.h"
#include "anbox/logger.h"
#include "anbox/utils.h"

#include <map>
#include <stdexcept>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/throw_exception.hpp>

#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace fs = boost::filesystem;

namespace {
constexpr unsigned int unprivileged_user_id{100000};
constexpr const char *default_container_ip_address{"192.168.250.2"};
constexpr const std::uint32_t default_container_ip_prefix_length{24};
constexpr const char *default_host_ip_address{"192.168.250.1"};
constexpr const char *default_dns_server{"8.8.8.8"};
}

namespace anbox {
namespace container {
LxcContainer::LxcContainer(bool privileged, const network::Credentials &creds)
    : state_(State::inactive), container_(nullptr),  privileged_(privileged), creds_(creds) {
  utils::ensure_paths({
      SystemConfiguration::instance().container_config_dir(),
      SystemConfiguration::instance().log_dir(),
  });
}

LxcContainer::~LxcContainer() {
  stop();
  if (container_) lxc_container_put(container_);
}

void LxcContainer::setup_id_maps() {
  const auto base_id = unprivileged_user_id;
  const auto max_id = 65536;

  set_config_item("lxc.id_map",
                  utils::string_format("u 0 %d %d", base_id, creds_.uid() - 1));
  set_config_item("lxc.id_map",
                  utils::string_format("g 0 %d %d", base_id, creds_.gid() - 1));

  // We need to bind the user id for the one running the client side
  // process as he is the owner of various socket files we bind mount
  // into the container.
  set_config_item("lxc.id_map",
                  utils::string_format("u %d %d 1", creds_.uid(), creds_.uid()));
  set_config_item("lxc.id_map",
                  utils::string_format("g %d %d 1", creds_.gid(), creds_.gid()));

  set_config_item("lxc.id_map",
                  utils::string_format("u %d %d %d", creds_.uid() + 1,
                                       base_id + creds_.uid() + 1,
                                       max_id - creds_.uid() - 1));
  set_config_item("lxc.id_map",
                  utils::string_format("g %d %d %d", creds_.uid() + 1,
                                       base_id + creds_.gid() + 1,
                                       max_id - creds_.gid() - 1));
}

void LxcContainer::setup_network() {
  if (!fs::exists("/sys/class/net/anbox0")) {
    WARNING("Anbox bridge interface 'anbox0' doesn't exist. Network functionality will not be available");
    return;
  }

  set_config_item("lxc.network.type", "veth");
  set_config_item("lxc.network.flags", "up");
  set_config_item("lxc.network.link", "anbox0");

  // Instead of relying on DHCP we will give Android a static IP configuration
  // for the virtual ethernet interface LXC creates for us. This will be bridged
  // to the host and will allows us to have reliable network connectivity and
  // not depend on any other system service.

  android::IpConfigBuilder ip_conf;
  ip_conf.set_version(android::IpConfigBuilder::Version::Version2);
  ip_conf.set_assignment(android::IpConfigBuilder::Assignment::Static);
  ip_conf.set_link_address(default_container_ip_address, default_container_ip_prefix_length);
  ip_conf.set_gateway(default_host_ip_address);
  ip_conf.set_dns_servers({default_dns_server});
  ip_conf.set_id(0);

  std::vector<std::uint8_t> buffer(512);
  common::BinaryWriter writer(buffer.begin(), buffer.end());
  const auto size = ip_conf.write(writer);

  const auto data_ethernet_path = fs::path("data") / "misc" / "ethernet";
  const auto ip_conf_dir = SystemConfiguration::instance().data_dir() / data_ethernet_path;
  if (!fs::exists(ip_conf_dir)) {
    fs::create_directories(ip_conf_dir);

    // We have to walk through the created directory hierachy now and
    // ensure the permissions are set correctly. Otherwise the Android
    // system will fail to boot as it isn't allowed to write anything
    // into these directories.
    for (auto iter = data_ethernet_path.begin(); iter != data_ethernet_path.end(); iter++) {
      const auto path = SystemConfiguration::instance().data_dir() / *iter;
      if (::chown(path.c_str(), unprivileged_user_id, unprivileged_user_id) < 0)
        WARNING("Failed to set owner for path '%s'", path);
    }
  }

  const auto ip_conf_path = ip_conf_dir / "ipconfig.txt";
  if (fs::exists(ip_conf_path))
    fs::remove(ip_conf_path);

  std::ofstream f(ip_conf_path.string(), std::ofstream::binary);
  if (f.is_open()) {
    f.write(reinterpret_cast<const char*>(buffer.data()), size);
    f.close();
  } else {
    ERROR("Failed to write IP configuration. Network functionality will not be available.");
  }
}

void LxcContainer::start(const Configuration &configuration) {
  if (getuid() != 0)
    BOOST_THROW_EXCEPTION(std::runtime_error("You have to start the container as root"));

  if (container_ && container_->is_running(container_)) {
    WARNING("Container already started, stopping it now");
    container_->stop(container_);
  }

  if (!container_) {
    const auto container_config_dir = SystemConfiguration::instance().container_config_dir();
    DEBUG("Containers are stored in %s", container_config_dir);

    // Remove container config to be be able to rewrite it
    ::unlink(utils::string_format("%s/default/config", container_config_dir).c_str());

    container_ = lxc_container_new("default", container_config_dir.c_str());
    if (!container_)
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create LXC container instance"));

    // If container is still running (for example after a crash) we stop it here
    // to ensure
    // its configuration is synchronized.
    if (container_->is_running(container_)) container_->stop(container_);
  }

  // We can mount proc/sys as rw here as we will run the container unprivileged
  // in the end
  set_config_item("lxc.mount.auto", "proc:mixed sys:mixed cgroup:mixed");

  set_config_item("lxc.autodev", "1");
  set_config_item("lxc.pts", "1024");
  set_config_item("lxc.tty", "0");
  set_config_item("lxc.utsname", "anbox");

  set_config_item("lxc.group.devices.deny", "");
  set_config_item("lxc.group.devices.allow", "");

  // We can't move bind-mounts, so don't use /dev/lxc/
  set_config_item("lxc.devttydir", "");

  set_config_item("lxc.environment",
                  "PATH=/system/bin:/system/sbin:/system/xbin");

  set_config_item("lxc.init_cmd", "/anbox-init.sh");
  set_config_item("lxc.rootfs.backend", "dir");

  const auto rootfs_path = SystemConfiguration::instance().rootfs_dir();
  DEBUG("Using rootfs path %s", rootfs_path);
  set_config_item("lxc.rootfs", rootfs_path);

  set_config_item("lxc.loglevel", "0");
  const auto log_path = SystemConfiguration::instance().log_dir();
  set_config_item("lxc.logfile", utils::string_format("%s/container.log", log_path).c_str());

  setup_network();

#if 0
    // Android uses namespaces as well so we have to allow nested namespaces for LXC
    // which are otherwise forbidden by AppArmor.
    set_config_item("lxc.aa_profile", "lxc-container-default-with-nesting");
#else
  // FIXME: when using the nested profile we still get various denials from
  // things Android tries to do but isn't allowed to. We need to look into
  // those and see how we can switch back to a confined way of running the
  // container.
  set_config_item("lxc.aa_profile", "unconfined");
#endif

  if (!privileged_)
    setup_id_maps();

  auto bind_mounts = configuration.bind_mounts;

  // Extra bind-mounts for user-namespace setup
  bind_mounts.insert({"/dev/console", "dev/console"});
  bind_mounts.insert({"/dev/full", "dev/full"});
  bind_mounts.insert({"/dev/null", "dev/null"});
  bind_mounts.insert({"/dev/random", "dev/random"});
  bind_mounts.insert({"/dev/tty", "dev/tty"});
  bind_mounts.insert({"/dev/urandom", "dev/urandom"});
  bind_mounts.insert({"/dev/zero", "dev/zero"});

  for (const auto &bind_mount : bind_mounts) {
    std::string create_type = "file";

    if (fs::is_directory(bind_mount.first)) create_type = "dir";

    auto target_path = bind_mount.second;
    // The target path needs to be absolute and pointing to the right
    // location inside the target rootfs as otherwise we get problems
    // when running in confined environments like snap's.
    if (!utils::string_starts_with(target_path, "/"))
      target_path = std::string("/") + target_path;
    target_path = rootfs_path + target_path;

    set_config_item(
        "lxc.mount.entry",
        utils::string_format("%s %s none bind,create=%s,optional 0 0",
                             bind_mount.first, target_path, create_type));
  }

  if (!container_->save_config(container_, nullptr))
    BOOST_THROW_EXCEPTION(
        std::runtime_error("Failed to save container configuration"));

  if (not container_->start(container_, 0, nullptr))
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to start container"));

  state_ = Container::State::running;

  DEBUG("Container successfully started");
}

void LxcContainer::stop() {
  if (not container_ || not container_->is_running(container_))
    return;

  if (not container_->stop(container_))
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to stop container"));

  state_ = Container::State::inactive;

  DEBUG("Container successfully stopped");
}

void LxcContainer::set_config_item(const std::string &key,
                                   const std::string &value) {
  if (!container_->set_config_item(container_, key.c_str(), value.c_str()))
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to configure LXC container"));
}

Container::State LxcContainer::state() { return state_; }
}  // namespace container
}  // namespace anbox
