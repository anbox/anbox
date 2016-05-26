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

#ifndef CORE_POSIX_WAIT_H_
#define CORE_POSIX_WAIT_H_

#include <core/posix/exit.h>
#include <core/posix/signal.h>
#include <core/posix/visibility.h>

#include <bitset>

#include <cstdint>

#include <sys/wait.h>

namespace core
{
namespace posix
{
namespace wait
{

/**
 * @brief Flags enumerates different behavior when waiting for a child process to change state.
 */
enum class Flags : std::uint8_t
{
    continued = WCONTINUED, ///< Also wait for a child to continue after having been stopped.
    untraced = WUNTRACED, ///< Also wait for state changes in untraced children.
    no_hang = WNOHANG ///< Do not block if a child process hasn't changed state.
};

CORE_POSIX_DLL_PUBLIC Flags operator|(Flags l, Flags r);

/**
 * @brief The Result struct encapsulates the result of waiting for a process state change.
 */
struct CORE_POSIX_DLL_PUBLIC Result
{
    /**
     * @brief The status of the process/wait operation.
     */
    enum class Status
    {
        undefined, ///< Marks an undefined state.
        no_state_change, ///< No state change occured.
        exited, ///< The process exited normally.
        signaled, ///< The process was signalled and terminated.
        stopped, ///< The process was signalled and stopped.
        continued ///< The process resumed operation.
    } status = Status::undefined;

    /**
     * @brief Union of result-specific details.
     */
    union
    {
        /**
         * Contains the exit status of the process if status == Status::exited.
         */
        struct
        {
            exit::Status status; ///< Exit status of the process.
        } if_exited;

        /**
         * Contains the signal that caused the process to terminate if status == Status::signaled.
         */
        struct
        {
            Signal signal; ///< Signal that caused the process to terminate.
            bool core_dumped; ///< true if the process termination resulted in a core dump.
        } if_signaled;

        /**
         * Contains the signal that caused the process to terminate if status == Status::stopped.
         */
        struct
        {
            Signal signal; ///< Signal that caused the process to terminate.
        } if_stopped;
    } detail;
};
}
}
}

#endif // CORE_POSIX_WAIT_H_
