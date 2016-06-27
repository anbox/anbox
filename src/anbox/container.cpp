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

#include <unistd.h>

#include <boost/filesystem.hpp>

#include <fstream>
#include <sstream>

#include "core/posix/fork.h"
#include "core/posix/exec.h"

#include "anbox/utils.h"
#include "anbox/logger.h"
#include "anbox/config.h"
#include "anbox/container.h"
#include "anbox/common/fd.h"

#include <grp.h>
#include <sys/mount.h>

namespace fs = boost::filesystem;
namespace anbox {

Container::Spec Container::Spec::Default() {
    Spec spec;
    spec.init_command = "/anbox-init.sh";
    spec.environment.insert({"PATH", "/system/bin:/system/sbin:/system/xbin"});
    return spec;
}
std::shared_ptr<Container> Container::create(const Container::Spec &spec) {
    return std::shared_ptr<Container>(new Container{spec});
}

Container::Container(const Container::Spec &spec) :
    spec_(spec),
    child_(core::posix::ChildProcess::invalid()),
    child_group_(core::posix::ProcessGroup::invalid()) {
}

Container::~Container() {
    stop();
}

void Container::start() {
    std::vector<std::string> args = {
        // We need to setup user mapping here as lxc-usernsexec will not
        // map our current user to root which we need to allow our container
        // to access files we've created and mapped into it
        "-m", utils::string_format("u:0:%d:1", getuid()),
        "-m", utils::string_format("g:0:%d:1", getgid()),
        // For all other users inside the container we're using a subuid/
        // subgid range which is defined on the host.
        "-m", "u:1:100000:65536",
        "-m", "g:1:100000:65536",
        "--",
        // FIXME(morphis): use system or in-click path
        "/home/phablet/anbox-container",
        "--bind", spec_.rootfs_path, "/",
        "--dev", "/dev",
        "--proc", "/proc",
        "--unshare-ipc",
        "--unshare-pid",
        "--unshare-uts",
        "--chdir", "/",
        "--pid-file", utils::string_format("%s/pid", config::data_path()),
    };

    for (const auto &dir : spec_.temporary_dirs) {
        args.push_back("--tmpfs");
        args.push_back(dir);
    }

    for (const auto &path : spec_.dev_bind_paths) {
        args.push_back("--dev-bind");
        args.push_back(path);
        args.push_back(path);
    }

    for (const auto &path : spec_.bind_paths) {
        args.push_back("--bind");
        args.push_back(path.first);
        args.push_back(path.second);
    }

    for (const auto &env : spec_.environment) {
        args.push_back("--setenv");
        args.push_back(env.first);
        args.push_back(env.second);
    }

    args.push_back(spec_.init_command);

    std::map<std::string,std::string> env = {
        // lxc-usernsexec needs this as otherwise it doesn't find the
        // newuidmap/newgidmap utilities it uses to setup the user
        // namespace
        { "PATH", "/usr/bin" },
    };

    child_ = core::posix::exec("/usr/bin/lxc-usernsexec", args, env, core::posix::StandardStream::empty);
    child_group_ = child_.process_group_or_throw();
}

void Container::stop() {
    child_group_.send_signal_or_throw(core::posix::Signal::sig_kill);
}

} // namespace anbox
