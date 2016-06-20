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

#include "anbox/container_connector.h"
#include "anbox/namespace_attacher.h"
#include "anbox/config.h"
#include "anbox/utils.h"
#include "anbox/logger.h"

#include "core/posix/exec.h"

#include <boost/filesystem.hpp>

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <grp.h>

namespace fs = boost::filesystem;

namespace anbox {
ContainerConnector::ContainerConnector(int pid) :
    pid_(pid) {
}

ContainerConnector::~ContainerConnector() {
}

int ContainerConnector::run(const std::string &path) {
    int pid = pid_;
    if (pid == -1)
        pid = std::stol(utils::read_file_if_exists_or_throw(
                                    utils::string_format("%s/pid", config::data_path())));

    if (!fs::is_directory(fs::path(utils::string_format("/proc/%i", pid))))
        BOOST_THROW_EXCEPTION(std::runtime_error("Container isn't running"));

    namespaces_ = std::make_shared<NamespaceAttacher>(std::vector<NamespaceType>{
                                                          NamespaceType::user,
                                                          NamespaceType::mount,
                                                          NamespaceType::pid,
                                                          NamespaceType::uts,
                                                          NamespaceType::ipc,
                                                          NamespaceType::net,
                                                      }, pid);

    // A few things we want to preset in our env within the container shell
    std::map<std::string, std::string> env = {
        { "ANDROID_ROOT", "/system" },
        { "ANDROID_DATA", "/data" },
    };

    auto child = core::posix::exec(path, {}, env, core::posix::StandardStream::empty, [this]() {
        // We're now in the namespace bwrap spawns up for the container which has
        // a /newroot directory pointing to the new root filesystem we get from
        // Android and is what we need to change to in order to work within
        // the Anroid system.
        if (::chroot("/newroot") != 0)
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to enter container root filesystem"));

        setuid(0);
        setgid(0);
        setgroups(0, nullptr);

        chdir("/");
    });

    child.wait_for(core::posix::wait::Flags::untraced);

    return EXIT_SUCCESS;
}

} // namespace
