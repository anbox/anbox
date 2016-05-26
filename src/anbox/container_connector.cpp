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
#include "anbox/config.h"
#include "anbox/utils.h"
#include "anbox/logger.h"

#include "core/posix/exec.h"

#include <boost/filesystem.hpp>

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace fs = boost::filesystem;

namespace {
class NamespaceAttacher {
public:
    enum class ns_type {
        user,
        pid,
        uts,
        mount,
        ipc,
        net,
    };

    static std::string ns_type_to_string(ns_type type) {
        switch (type) {
        case ns_type::user:
            return "user";
        case ns_type::pid:
            return "pid";
        case ns_type::uts:
            return "uts";
        case ns_type::mount:
            return "mnt";
        case ns_type::ipc:
            return "ipc";
        case ns_type::net:
            return "net";
        default:
            break;
        }

        BOOST_THROW_EXCEPTION(std::runtime_error("Unknown namespace type"));
    }

    NamespaceAttacher(const std::vector<ns_type> &types, pid_t pid) :
        pid_(pid) {

        attach(types);
    }

    ~NamespaceAttacher() {
    }

private:
    void attach(const std::vector<ns_type> &types) {
        std::vector<int> fds;
        for (const auto &type : types) {
            const auto path = anbox::utils::string_format("/proc/%lu/ns/%s", pid_, ns_type_to_string(type));
            if (!fs::exists(fs::path(path)))
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open namespace file"));

            const auto fd = ::open(path.c_str(), O_RDONLY);
            if (fd < 0)
                BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open namespace file"));

            fds.push_back(fd);
        }

        for (const auto &fd : fds) {
            if (::setns(fd, 0) == -1)
                BOOST_THROW_EXCEPTION(std::runtime_error("Could not attach to namespace"));

            ::close(fd);
        }
    }

    pid_t pid_;
};
}

namespace anbox {
ContainerConnector::ContainerConnector() {
}

ContainerConnector::~ContainerConnector() {
}

int ContainerConnector::run(const std::string &path) {
    const auto pid = std::stol(utils::read_file_if_exists_or_throw(
                                    utils::string_format("%s/pid", config::data_path())));

    if (!fs::is_directory(fs::path(utils::string_format("/proc/%i", pid))))
        BOOST_THROW_EXCEPTION(std::runtime_error("Container isn't running"));

    namespaces_ = std::make_shared<NamespaceAttacher>(std::vector<NamespaceAttacher::ns_type>{
                                                          NamespaceAttacher::ns_type::user,
                                                          NamespaceAttacher::ns_type::mount,
                                                          NamespaceAttacher::ns_type::pid,
                                                          NamespaceAttacher::ns_type::uts,
                                                          NamespaceAttacher::ns_type::ipc,
                                                          NamespaceAttacher::ns_type::net,
                                                      }, pid);

    // A few things we want to preset in our env within the container shell
    std::map<std::string, std::string> env = {
        { "ANDROID_ROOT", "/" }
    };

    auto child = core::posix::exec(path, {}, env, core::posix::StandardStream::empty, [this]() {
        // We're now in the namespace bwrap spawns up for the container which has
        // a /newroot directory pointing to the new root filesystem we get from
        // Android and is what we need to change to in order to work within
        // the Anroid system.
        if (::chroot("/newroot") != 0)
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to enter container root filesystem"));

        chdir("/");
    });

    child.wait_for(core::posix::wait::Flags::untraced);

    return EXIT_SUCCESS;
}

} // namespace
