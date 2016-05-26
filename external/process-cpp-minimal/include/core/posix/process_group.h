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

#ifndef CORE_POSIX_PROCESS_GROUP_H_
#define CORE_POSIX_PROCESS_GROUP_H_

#include <core/posix/signalable.h>
#include <core/posix/visibility.h>

#include <memory>

namespace core
{
namespace posix
{
class Process;

/**
 * @brief The ProcessGroup class models a signalable group of process.
 *
 * Summary from http://en.wikipedia.org/wiki/Process_group:
 *
 * In POSIX-conformant operating systems, a process group denotes a collection
 * of one or more processes. Process groups are used to control the distribution
 * of signals. A signal directed to a process group is delivered individually to
 * all of the processes that are members of the group.
 */
class CORE_POSIX_DLL_PUBLIC ProcessGroup : public Signalable
{
public:
    /**
     * @brief Accesses the id of this process group.
     * @return The id of this process group.
     */
    virtual pid_t id() const;

    static ProcessGroup invalid();

protected:
    friend class Process;
    CORE_POSIX_DLL_LOCAL ProcessGroup(pid_t id);

private:
    struct CORE_POSIX_DLL_LOCAL Private;
    std::shared_ptr<Private> d;
};
}
}

#endif // CORE_POSIX_PROCESS_GROUP_H_
