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

#include "core/posix/fork.h"
#include "core/posix/exec.h"

#include "anbox/utils.h"
#include "anbox/logger.h"
#include "anbox/config.h"
#include "anbox/container.h"
#include "anbox/common/fd.h"

extern "C" int bwrap_main(int argc, char **argv);

namespace fs = boost::filesystem;
namespace anbox {

Container::Spec Container::Spec::Default() {
    Spec spec;
    spec.init_command = "/init";
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
    std::vector<std::string> arguments = {
        "--ro-bind", spec_.rootfs_path, "/",
        "--dev", "/dev",
        "--proc", "/proc",
        "--unshare-user",
        "--unshare-ipc",
        "--unshare-pid",
        "--unshare-net",
        "--unshare-uts",
        // We will take UID 0 (root) inside the container
        "--uid", "0",
        // We will take GID 0 (root) inside the container
        "--gid", "0",
        "--chdir", "/",
        "--pid-file", utils::string_format("%s/pid", config::data_path()),
    };

    for (const auto &dir : spec_.temporary_dirs) {
        arguments.push_back("--tmpfs");
        arguments.push_back(dir);
    }

    for (const auto &path : spec_.dev_bind_paths) {
        arguments.push_back("--dev-bind");
        arguments.push_back(path);
        arguments.push_back(path);
    }

    for (const auto &path : spec_.bind_paths) {
        arguments.push_back("--bind");
        arguments.push_back(path.first);
        arguments.push_back(path.second);
    }

    for (const auto &env : spec_.environment) {
        arguments.push_back("--setenv");
        arguments.push_back(env.first);
        arguments.push_back(env.second);
    }

    arguments.push_back(spec_.init_command);

    child_ = core::posix::fork([&]() {
        char **it, **pargv;
        it = pargv = new char*[arguments.size() + 2];
        *it = strdup("bwrap");
        it++;
        for (auto arg : arguments) {
            *it = ::strdup(arg.c_str());
            it++;
        }
        *it = nullptr;

        if (bwrap_main(arguments.size() + 1, pargv))
            return core::posix::exit::Status::failure;

        return core::posix::exit::Status::success;
    }, core::posix::StandardStream::empty);

    child_group_ = child_.process_group_or_throw();
}

void Container::stop() {
    child_group_.send_signal_or_throw(core::posix::Signal::sig_kill);
}

} // namespace anbox
