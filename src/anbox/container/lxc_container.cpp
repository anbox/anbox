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
#include "anbox/common/binder_device_allocator.h"
#include "anbox/common/binder_device.h"
#include "anbox/container/lxc_container.h"
#include "anbox/system_configuration.h"
#include "anbox/logger.h"
#include "anbox/utils.h"

#include <map>
#include <stdexcept>
#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/throw_exception.hpp>

#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <unistd.h>

namespace fs = boost::filesystem;

namespace {
constexpr unsigned int unprivileged_uid{100000};
constexpr unsigned int android_system_uid{1000};
constexpr const char *default_container_ip_address{"192.168.250.2"};
constexpr const std::uint32_t default_container_ip_prefix_length{24};
constexpr const char *default_host_ip_address{"192.168.250.1"};
constexpr const char *default_dns_server{"8.8.8.8"};
constexpr int num_needed_binders{1};

#ifdef ENABLE_LXC2_SUPPORT
constexpr const char *lxc_config_idmap_key{"lxc.id_map"};
constexpr const char *lxc_config_net_type_key{"lxc.network.type"};
constexpr const char *lxc_config_net_flags_key{"lxc.network.flags"};
constexpr const char *lxc_config_net_link_key{"lxc.network.link"};
constexpr const char *lxc_config_pty_max_key{"lxc.pts"};
constexpr const char *lxc_config_tty_max_key{"lxc.tty"};
constexpr const char *lxc_config_uts_name_key{"lxc.utsname"};
constexpr const char *lxc_config_tty_dir_key{"lxc.devttydir"};
constexpr const char *lxc_config_init_cmd_key{"lxc.init_cmd"};
constexpr const char *lxc_config_rootfs_path_key{"lxc.rootfs"};
constexpr const char *lxc_config_log_level_key{"lxc.loglevel"};
constexpr const char *lxc_config_log_file_key{"lxc.logfile"};
constexpr const char *lxc_config_apparmor_profile_key{"lxc.aa_profile"};
#else
constexpr const char *lxc_config_idmap_key{"lxc.idmap"};
constexpr const char *lxc_config_net_type_key{"lxc.net.0.type"};
constexpr const char *lxc_config_net_flags_key{"lxc.net.0.flags"};
constexpr const char *lxc_config_net_link_key{"lxc.net.0.link"};
constexpr const char *lxc_config_pty_max_key{"lxc.pty.max"};
constexpr const char *lxc_config_tty_max_key{"lxc.tty.max"};
constexpr const char *lxc_config_uts_name_key{"lxc.uts.name"};
constexpr const char *lxc_config_tty_dir_key{"lxc.tty.dir"};
constexpr const char *lxc_config_init_cmd_key{"lxc.init.cmd"};
constexpr const char *lxc_config_rootfs_path_key{"lxc.rootfs.path"};
constexpr const char *lxc_config_log_level_key{"lxc.log.level"};
constexpr const char *lxc_config_log_file_key{"lxc.log.file"};
constexpr const char *lxc_config_apparmor_profile_key{"lxc.apparmor.profile"};
#endif

constexpr int device_major(dev_t dev) {
  return int(((dev >> 8) & 0xfff) | ((dev >> 32) & (0xfffff000)));
}

constexpr int device_minor(dev_t dev) {
  return int((dev & 0xff) | ((dev >> 12) & (0xffffff00)));
}
} // namespace

namespace anbox {
namespace container {
LxcContainer::LxcContainer(bool privileged,
                           bool rootfs_overlay,
                           const std::string& container_network_address,
                           const std::string &container_network_gateway,
                           const std::vector<std::string> &container_network_dns_servers,
                           const network::Credentials &creds)
    : state_(State::inactive),
      container_(nullptr),
      privileged_(privileged),
      rootfs_overlay_(rootfs_overlay),
      container_network_address_(container_network_address),
      container_network_gateway_(container_network_gateway),
      container_network_dns_servers_(container_network_dns_servers),
      creds_(creds) {
  utils::ensure_paths({
      SystemConfiguration::instance().container_config_dir(),
      SystemConfiguration::instance().container_state_dir(),
      SystemConfiguration::instance().log_dir(),
  });
}

LxcContainer::~LxcContainer() {
  stop();
  if (container_)
    lxc_container_put(container_);
}

void LxcContainer::setup_id_map() {
  const auto base_id = unprivileged_uid;
  const auto max_id = 100000;

  set_config_item(lxc_config_idmap_key, utils::string_format("u 0 %d %d", base_id, android_system_uid - 1));
  set_config_item(lxc_config_idmap_key, utils::string_format("g 0 %d %d", base_id, android_system_uid - 1));

  // We need to bind the user id for the one running the client side
  // process as he is the owner of various socket files we bind mount
  // into the container.
  set_config_item(lxc_config_idmap_key, utils::string_format("u %d %d 1", android_system_uid, creds_.uid()));
  set_config_item(lxc_config_idmap_key, utils::string_format("g %d %d 1", android_system_uid, creds_.gid()));

  set_config_item(lxc_config_idmap_key, utils::string_format("u %d %d %d", android_system_uid + 1,
                                                     base_id + android_system_uid + 1,
                                                     max_id - creds_.uid() - 1));
  set_config_item(lxc_config_idmap_key, utils::string_format("g %d %d %d", android_system_uid + 1,
                                                     base_id + android_system_uid + 1,
                                                     max_id - creds_.gid() - 1));
}

void LxcContainer::setup_network() {
  if (!fs::exists("/sys/class/net/anbox0")) {
    WARNING("Anbox bridge interface 'anbox0' doesn't exist. Network functionality will not be available");
    return;
  }

  set_config_item(lxc_config_net_type_key, "veth");
  set_config_item(lxc_config_net_flags_key, "up");
  set_config_item(lxc_config_net_link_key, "anbox0");

  // Instead of relying on DHCP we will give Android a static IP configuration
  // for the virtual ethernet interface LXC creates for us. This will be bridged
  // to the host and will allows us to have reliable network connectivity and
  // not depend on any other system service.

  android::IpConfigBuilder ip_conf;
  ip_conf.set_version(android::IpConfigBuilder::Version::Version2);
  ip_conf.set_assignment(android::IpConfigBuilder::Assignment::Static);

  std::string address = default_container_ip_address;
  std::uint32_t ip_prefix_length = default_container_ip_prefix_length;
  if (!container_network_address_.empty()) {
    auto tokens = utils::string_split(container_network_address_, '/');
    if (tokens.size() == 1 || tokens.size() == 2)
      address = tokens[0];
    if (tokens.size() == 2)
      ip_prefix_length = atoi(tokens[1].c_str());
  }
  ip_conf.set_link_address(address, ip_prefix_length);

  std::string gateway = default_host_ip_address;
  if (!container_network_gateway_.empty())
    gateway = container_network_gateway_;
  ip_conf.set_gateway(gateway);

  if (container_network_dns_servers_.size() > 0)
    ip_conf.set_dns_servers(container_network_dns_servers_);
  else
    ip_conf.set_dns_servers({default_dns_server});

  ip_conf.set_id(0);

  std::vector<std::uint8_t> buffer(512);
  common::BinaryWriter writer(buffer.begin(), buffer.end());
  const auto size = ip_conf.write(writer);

  const auto data_ethernet_path = fs::path("data") / "misc" / "ethernet";
  const auto ip_conf_dir = SystemConfiguration::instance().data_dir() / data_ethernet_path;
  if (!fs::exists(ip_conf_dir))
    fs::create_directories(ip_conf_dir);

  // We have to walk through the created directory hierachy now and
  // ensure the permissions are set correctly. Otherwise the Android
  // system will fail to boot as it isn't allowed to write anything
  // into these directories. As previous versions of Anbox which were
  // published to our users did this incorrectly we need to check on
  // every startup if those directories are still owned by root and
  // if they are we move them over to the unprivileged user.
  auto path = SystemConfiguration::instance().data_dir();
  for (auto iter = data_ethernet_path.begin(); iter != data_ethernet_path.end(); iter++) {
    path /= *iter;

    struct stat st;
    if (stat(path.c_str(), &st) < 0) {
      WARNING("Cannot retrieve permissions of path %s", path);
      continue;
    }

    if (st.st_uid != 0 && st.st_gid != 0)
      continue;

    if (::chown(path.c_str(), unprivileged_uid, unprivileged_uid) < 0)
      WARNING("Failed to set owner for path '%s'", path);
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

void LxcContainer::add_device(const std::string& device, const DeviceSpecification& spec) {
  struct stat st;
  const std::string *old_device_name;
  if (!spec.old_device_name.empty())
    old_device_name = &spec.old_device_name;
  else
    old_device_name = &device;
  int r = stat(old_device_name->c_str(), &st);
  if (r < 0) {
    const auto msg = utils::string_format("Failed to retrieve information about device %s", device);
    throw std::runtime_error(msg);
  }

  const auto major = device_major(st.st_rdev);
  const auto minor = device_minor(st.st_rdev);
  const auto mode = ((st.st_mode >> 9) << 9) | (spec.permission & ~(1 << 9));
  const auto new_device_name = fs::basename(device);
  const auto devices_path = fs::path(SystemConfiguration::instance().container_devices_dir());
  const auto new_device_path = (devices_path / new_device_name).string();

  const auto encoded_device_number = (minor & 0xff) | (major << 8) | ((minor & !0xff) << 12);
  r = mknod(new_device_path.c_str(), mode, encoded_device_number);
  if (r < 0) {
    auto msg = utils::string_format("Failed to create node for device %s: %s",
                                    device, strerror(errno));
    throw std::runtime_error(msg);
  }

  auto base_uid = unprivileged_uid;
  if (privileged_)
    base_uid = 0;

  const auto shifted_uid = base_uid + st.st_uid;
  const auto shifted_gid = base_uid + st.st_gid;
  r = chown(new_device_path.c_str(), shifted_uid, shifted_gid);
  if (r < 0) {
    auto msg = utils::string_format("Failed to change ownership of new node for %s: %s",
                                    device, strerror(errno));
    throw std::runtime_error(msg);
  }

  // Needed as mknod respects the umask
  r = chmod(new_device_path.c_str(), mode);
  if (r < 0) {
    auto msg = utils::string_format("Failed to change mode of new node for %s: %s",
                                    device, strerror(errno));
    throw::std::runtime_error(msg);
  }

  auto target_path = device;
  // Strip a leading slash as LXC doesn't like that
  if (utils::string_starts_with(device, "/"))
    target_path = device.substr(1, device.length() - 1);

  const auto entry = utils::string_format("%s %s none bind,create=file,optional 0 0",
                                          new_device_path, target_path);
  set_config_item("lxc.mount.entry", entry);
}

bool LxcContainer::create_binder_devices(unsigned int device_count, std::vector<std::unique_ptr<common::BinderDevice>>& devices) {
  // We will always allocate a static set of binders devices even if the container
  // doesn't use all of them
  for (unsigned int n = 0; n < device_count; n++) {
    auto device = common::BinderDeviceAllocator::new_device();
    if (!device)
      return false;

    DEBUG("Allocated binder device %s", device->path());
    devices.push_back(std::move(device));
  }

  return true;
}

void LxcContainer::start(const Configuration &configuration) {
  if (getuid() != 0)
    throw std::runtime_error("You have to start the container as root");

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
      throw std::runtime_error("Failed to create LXC container instance");

    // If container is still running (for example after a crash) we stop it here
    // to ensure its configuration is synchronized.
    if (container_->is_running(container_))
      container_->stop(container_);
  }

  // We can mount proc/sys as rw here as we will run the container unprivileged
  // in the end
  set_config_item("lxc.mount.auto", "proc:mixed sys:mixed cgroup:mixed");

  set_config_item("lxc.autodev", "1");
  set_config_item(lxc_config_pty_max_key, "1024");
  set_config_item(lxc_config_tty_max_key, "0");
  set_config_item(lxc_config_uts_name_key, "anbox");

  set_config_item("lxc.group.devices.deny", "");
  set_config_item("lxc.group.devices.allow", "");

  // We can't move bind-mounts, so don't use /dev/lxc/
  set_config_item(lxc_config_tty_dir_key, "");

  set_config_item("lxc.environment", "PATH=/system/bin:/system/sbin:/system/xbin");

  set_config_item(lxc_config_init_cmd_key, "/anbox-init.sh");

#ifdef ENABLE_SNAP_CONFINEMENT
  // If we're running inside the snap environment snap-confine already created a
  // cgroup for us we need to use as otherwise presevering a namespace wont help.
  if (utils::is_env_set("SNAP"))
    set_config_item("lxc.namespace.keep", "cgroup");
#endif

  auto rootfs_path = SystemConfiguration::instance().rootfs_dir();
  if (rootfs_overlay_)
    rootfs_path = SystemConfiguration::instance().combined_rootfs_dir();

  DEBUG("Using rootfs path %s", rootfs_path);
  set_config_item(lxc_config_rootfs_path_key, rootfs_path);

  set_config_item(lxc_config_log_level_key, "0");
  const auto log_path = SystemConfiguration::instance().log_dir();
  set_config_item(lxc_config_log_file_key, utils::string_format("%s/container.log", log_path).c_str());

#ifndef ENABLE_LXC2_SUPPORT
    // Dump the console output to disk to have a chance to debug early boot problems
    set_config_item("lxc.console.logfile", utils::string_format("%s/console.log", log_path).c_str());
    set_config_item("lxc.console.rotate", "1");
#endif

  setup_network();

#ifdef ENABLE_SNAP_CONFINEMENT
  // We take the AppArmor profile snapd has defined for us as part of the
  // anbox-support interface. The container manager itself runs within a
  // child profile snap.anbox.container-manager//lxc too.
  set_config_item("lxc.apparmor.profile", "snap.anbox.container-manager//container");
#else
  set_config_item(lxc_config_apparmor_profile_key, "unconfined");
#endif

  if (!privileged_)
    setup_id_map();

  auto bind_mounts = configuration.bind_mounts;
  auto devices = configuration.devices;

  // If we have binderfs support we can dynamically allocate all our devices
  if (common::BinderDeviceAllocator::is_supported()) {
    DEBUG("Using binderfs to allocate our own binder nodes");

    std::vector<std::unique_ptr<common::BinderDevice>> binder_devices;
    if (!create_binder_devices(num_needed_binders, binder_devices) ||
        binder_devices.size() != num_needed_binders)
      throw std::runtime_error("Failed to allocate necessary binder devices");

    bind_mounts.insert({binder_devices[0]->path().string(), "/dev/binder"});
    binder_devices_ = std::move(binder_devices);
  } else {
    DEBUG("Using static binder device /dev/binder");
    devices.insert({"/dev/binder", { 0666 }});
  }

  for (const auto &bind_mount : bind_mounts) {
    std::string create_type = "file";

    if (fs::is_directory(bind_mount.first))
      create_type = "dir";

    auto target_path = bind_mount.second;
    // The target path needs to be absolute and pointing to the right
    // location inside the target rootfs as otherwise we get problems
    // when running in confined environments like snap's.
    if (!utils::string_starts_with(target_path, "/"))
      target_path = std::string("/") + target_path;
    target_path = rootfs_path + target_path;

    const auto entry = utils::string_format("%s %s none bind,create=%s,optional 0 0",
                                            bind_mount.first, target_path, create_type);
    set_config_item("lxc.mount.entry", entry);
  }

  // Additional devices we need in our container
  devices.insert({"/dev/console", {0600}});
  devices.insert({"/dev/full", {0666}});
  devices.insert({"/dev/null", {0666}});
  devices.insert({"/dev/random", {0666}});
  devices.insert({"/dev/tty", {0666}});
  devices.insert({"/dev/urandom", {0666}});
  devices.insert({"/dev/zero", {0666}});
  devices.insert({"/dev/tun", {0660, "/dev/net/tun"}});
  devices.insert({"/dev/ashmem", {0666}});

  // Remove all left over devices from last time first before
  // creating any new ones
  const auto devices_dir = SystemConfiguration::instance().container_devices_dir();
  fs::remove_all(devices_dir);
  fs::create_directories(devices_dir);

  for (const auto& device : devices)
    add_device(device.first, device.second);

  // If we have any additional properties we add them at the top of default.prop
  // within the Android rootfs which we overlay with a bind mount.
  if (configuration.extra_properties.size() > 0) {
    const auto container_state_dir = SystemConfiguration::instance().container_state_dir();
    auto old_default_prop_path = fs::path(rootfs_path) / "default.prop";
    auto new_default_prop_path = fs::path(container_state_dir) / "default.prop";
    auto default_prop_content = utils::read_file_if_exists_or_throw(old_default_prop_path.string());

    std::ofstream default_props;
    default_props.open(new_default_prop_path.string(), std::ios_base::out);
    if (!default_props.is_open())
      throw std::runtime_error("Failed to open new default properties file");

    default_props << "# Properties added by Anbox" << std::endl;
    for (const auto& prop : configuration.extra_properties)
      default_props << prop << std::endl;

    default_props << std::endl
                  << default_prop_content << std::endl;

    default_props.close();

    set_config_item("lxc.mount.entry",
                    utils::string_format("%s %s/default.prop none bind,optional,ro 0 0",
                                         new_default_prop_path.string(), rootfs_path));
  }

  if (!container_->save_config(container_, nullptr))
    throw std::runtime_error("Failed to save container configuration");

  if (!container_->start(container_, 0, nullptr))
    throw std::runtime_error("Failed to start container");

  state_ = Container::State::running;

  DEBUG("Container successfully started");
}

void LxcContainer::stop() {
  if (!container_ || !container_->is_running(container_))
    return;

  if (!container_->stop(container_))
    throw std::runtime_error("Failed to stop container");

  state_ = Container::State::inactive;
  binder_devices_.clear();

  DEBUG("Container successfully stopped");
}

void LxcContainer::set_config_item(const std::string &key,
                                   const std::string &value) {
  if (!container_->set_config_item(container_, key.c_str(), value.c_str())) {
    const auto msg = utils::string_format("Failed to set config item %s", key);
    throw std::runtime_error(msg);
  }
}

Container::State LxcContainer::state() { return state_; }
}  // namespace container
}  // namespace anbox
