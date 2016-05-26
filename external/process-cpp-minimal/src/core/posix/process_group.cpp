/*
 * Copyright © 2013 Canonical Ltd.
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

#include <core/posix/process_group.h>

namespace core
{
namespace posix
{
struct ProcessGroup::Private
{
    pid_t id;
};

pid_t ProcessGroup::id() const
{
    return d->id;
}

ProcessGroup::ProcessGroup(pid_t id)
    : Signalable(-id), // We rely on ::kill to deliver signals, thus negate the id (see man 2 kill).
      d(new Private{id})
{
}

ProcessGroup ProcessGroup::invalid()
{
    static const pid_t invalid_pid = 1;
    return ProcessGroup(invalid_pid);
}
}
}
