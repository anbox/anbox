/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#ifndef CORE_TESTING_CROSS_PROCESS_SYNC_H_
#define CORE_TESTING_CROSS_PROCESS_SYNC_H_

#include <core/posix/visibility.h>

#include <cstdint>

#include <chrono>
#include <stdexcept>

namespace core
{
namespace testing
{
/**
 * @brief A cross-process synchronization primitive that supports simple wait-condition-like scenarios.
 */
class CORE_POSIX_DLL_PUBLIC CrossProcessSync
{
  public:
    struct Error
    {
        Error() = delete;
        ~Error() = delete;

        /**
         * @brief Thrown if any of the *_for functions times out.
         */
        struct Timeout : public std::runtime_error
        {
            Timeout() : std::runtime_error("Timeout while waiting for event to happen.")
            {
            }
        };
    };

    /**
     * @brief Constructs a new sync element.
     */
    CrossProcessSync();

    /**
     * @brief Copy c'tor, duping the underlying fds.
     * @param rhs The instance to copy.
     */
    CrossProcessSync(const CrossProcessSync& rhs);

    /**
      * @brief Closes the underlying fds.
      */
    ~CrossProcessSync() noexcept;

    /**
     * @brief operator =, dup's the underlying fds.
     * @param rhs The instance to assign from.
     * @return A mutable reference to this instance.
     */
    CrossProcessSync& operator=(const CrossProcessSync& rhs);

    /**
     * @brief Try to signal the other side that we are ready for at most duration milliseconds.
     * @throw Error::Timeout in case of a timeout.
     * @throw std::system_error for problems with the underlying pipe.
     */
    void try_signal_ready_for(const std::chrono::milliseconds& duration);

    /**
     * @brief Wait for the other sides to signal readiness for at most duration milliseconds.
     * @return The number of ready signals that have been collected since creation.
     * @throw Error::Timeout in case of a timeout.
     * @throw std::system_error for problems with the underlying pipe.
     */
    std::uint32_t wait_for_signal_ready_for(const std::chrono::milliseconds& duration);

  private:
    int fds[2]; ///< The cross-process pipe.
    std::uint32_t counter; ///< Counts the number of times the sync has been signalled.
};
}
}
#endif // CORE_TESTING_CROSS_PROCESS_SYNC_H_
