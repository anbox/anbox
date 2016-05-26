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

#include <core/posix/process.h>

#include <core/posix/signal.h>

#include <sys/types.h>
#include <unistd.h>

#include <iostream>

namespace core
{
namespace posix
{

struct Process::Private
{
    pid_t pid;
};

Process Process::invalid()
{
    static const pid_t invalid_pid = 0;
    Process p(invalid_pid);
    p.d->pid = -1;

    return p;
}

Process::Process(pid_t pid)
    : Signalable(pid),
      d(new Private{pid})
{
    if (pid < 0)
        throw std::runtime_error("Cannot construct instance for invalid pid.");
}

Process::~Process() noexcept
{
}

pid_t Process::pid() const
{
    return d->pid;
}

ProcessGroup Process::process_group_or_throw() const
{
    pid_t pgid = ::getpgid(pid());

    if (pgid == -1)
        throw std::system_error(errno, std::system_category());

    return ProcessGroup(pgid);
}

ProcessGroup Process::process_group(std::error_code& se) const noexcept(true)
{
    pid_t pgid = ::getpgid(pid());

    if (pgid == -1)
    {
        se = std::error_code(errno, std::system_category());
    }

    return ProcessGroup(pgid);
}
}
}
