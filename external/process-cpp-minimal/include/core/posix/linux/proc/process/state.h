/*
 * Copyright © 2012-2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#ifndef CORE_POSIX_LINUX_PROC_PROCESS_STATE_H_
#define CORE_POSIX_LINUX_PROC_PROCESS_STATE_H_

#include <core/posix/visibility.h>

#include <iosfwd>

namespace core
{
namespace posix
{
namespace linux
{
namespace proc
{
namespace process
{
enum class State
{
    undefined = -1,
    running = 'R',
    sleeping = 'S',
    disk_sleep = 'D',
    zombie = 'Z',
    traced_or_stopped = 'T',
    paging = 'W'
};
}
}
}
}
}

#endif // CORE_POSIX_LINUX_PROC_PROCESS_STATE_H_
