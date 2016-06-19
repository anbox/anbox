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

#ifndef ANBOX_NAMESPACE_ATTACHER_H_
#define ANBOX_NAMESPACE_ATTACHER_H_

#include <memory>
#include <vector>
#include <string>

namespace anbox {
enum class NamespaceType {
    user,
    pid,
    uts,
    mount,
    ipc,
    net,
};

class NamespaceAttacher {
public:
    static std::string ns_type_to_string(NamespaceType type);

    NamespaceAttacher(const std::vector<NamespaceType> &types, pid_t pid);
    ~NamespaceAttacher();

private:
    void attach(const std::vector<NamespaceType> &types);

    pid_t pid_;
};
} // namespace

#endif
