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

#include "anbox/namespace_attacher.h"
#include "anbox/utils.h"

#include <boost/throw_exception.hpp>
#include <boost/filesystem.hpp>

#include <unistd.h>
#include <fcntl.h>

namespace fs = boost::filesystem;

namespace anbox {
std::string NamespaceAttacher::ns_type_to_string(NamespaceType type) {
    switch (type) {
    case NamespaceType::user:
        return "user";
    case NamespaceType::pid:
        return "pid";
    case NamespaceType::uts:
        return "uts";
    case NamespaceType::mount:
        return "mnt";
    case NamespaceType::ipc:
        return "ipc";
    case NamespaceType::net:
        return "net";
    default:
        break;
    }

    BOOST_THROW_EXCEPTION(std::runtime_error("Unknown namespace type"));
}

NamespaceAttacher::NamespaceAttacher(const std::vector<NamespaceType> &types, pid_t pid) :
    pid_(pid) {

    attach(types);
}

NamespaceAttacher::~NamespaceAttacher() {
}

void NamespaceAttacher::attach(const std::vector<NamespaceType> &types) {
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
} // namespace anbox
