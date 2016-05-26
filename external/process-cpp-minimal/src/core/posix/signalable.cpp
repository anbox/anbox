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

#include <core/posix/signalable.h>

namespace core
{
namespace posix
{
struct Signalable::Private
{
    pid_t pid;
};

Signalable::Signalable(pid_t pid) : d(new Private{pid})
{
}

void Signalable::send_signal_or_throw(Signal signal)
{
    auto result = ::kill(d->pid, static_cast<int>(signal));

    if (result == -1)
        throw std::system_error(errno, std::system_category());
}

void Signalable::send_signal(Signal signal, std::error_code& e) noexcept
{
    auto result = ::kill(d->pid, static_cast<int>(signal));

    if (result == -1)
    {
        e = std::error_code(errno, std::system_category());
    }
}
}
}
