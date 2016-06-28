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

#include <cstring>

#include "anbox/config.h"
#include "anbox/utils.h"

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace anbox {
namespace config {
std::string data_path() {
    static std::string path;
    if (path.length() == 0) {
        const auto home_path = utils::get_env_value("HOME", "/home/phablet");
        path = utils::string_format("%s/.local/share/anbox", home_path);
        fs::create_directories(fs::path(path));
    }
    return path;
}

std::string host_share_path() {
    static std::string path;
    if (path.length() == 0)
        path = utils::string_format("%s/share", data_path());
    return path;
}

std::string container_share_path() {
    return "/data/anbox-share";
}
} // namespace config
} // namespace anbox
