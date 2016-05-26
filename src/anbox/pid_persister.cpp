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

#include <fstream>

#include <boost/filesystem.hpp>

#include "anbox/config.h"
#include "anbox/utils.h"
#include "anbox/pid_persister.h"

namespace fs = boost::filesystem;

namespace anbox {

PidPersister::PidPersister() :
    path_(utils::string_format("%s/pid", config::data_path())) {
    if (fs::exists(fs::path(path_)))
        std::remove(path_.c_str());

    std::ofstream file;
    file.open(path_);
    file << getpid();
    file.close();
}

PidPersister::~PidPersister() {
    std::remove(path_.c_str());
}

} // namespace anbox
