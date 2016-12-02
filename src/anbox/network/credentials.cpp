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

#include "anbox/network/credentials.h"

namespace anbox {
namespace network {
Credentials::Credentials(pid_t pid, uid_t uid, gid_t gid)
    : pid_{pid}, uid_{uid}, gid_{gid} {}

pid_t Credentials::pid() const { return pid_; }

uid_t Credentials::uid() const { return uid_; }

gid_t Credentials::gid() const { return gid_; }
}  // namespace network
}  // namespace anbox
