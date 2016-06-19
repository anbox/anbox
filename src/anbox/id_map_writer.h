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

#ifndef ANBOX_ID_MAP_WRITER_H_
#define ANBOX_ID_MAP_WRITER_H_

#include <vector>
#include <boost/filesystem.hpp>

namespace anbox {
class IdMapWriter {
public:
    enum class Type {
        UID,
        GID
    };

    IdMapWriter(const Type &type, const pid_t &pid = -1);
    ~IdMapWriter();

    void apply();

private:
    Type type_;
    pid_t pid_;
};
} // namespace anbox

#endif
