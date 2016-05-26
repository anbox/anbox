/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
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

#include <core/testing/cross_process_sync.h>

#include <system_error>

#include <poll.h>
#include <unistd.h>

namespace
{
const int read_fd = 0;
const int write_fd = 1;
}

core::testing::CrossProcessSync::CrossProcessSync() : counter(0)
{
    if (::pipe(fds) < 0)
        throw std::system_error(errno, std::system_category());
}

core::testing::CrossProcessSync::CrossProcessSync(const CrossProcessSync& rhs) : counter(rhs.counter)
{
    fds[0] = ::dup(rhs.fds[0]);
    fds[1] = ::dup(rhs.fds[1]);
}

core::testing::CrossProcessSync::~CrossProcessSync() noexcept
{
    ::close(fds[0]);
    ::close(fds[1]);
}

core::testing::CrossProcessSync& core::testing::CrossProcessSync::operator=(const core::testing::CrossProcessSync& rhs)
{
    ::close(fds[0]);
    ::close(fds[1]);
    fds[0] = ::dup(rhs.fds[0]);
    fds[1] = ::dup(rhs.fds[1]);

    counter = rhs.counter;

    return *this;
}

void core::testing::CrossProcessSync::try_signal_ready_for(const std::chrono::milliseconds& duration)
{
    static const short empty_revents = 0;
    pollfd poll_fd[1] = { { fds[write_fd], POLLOUT, empty_revents } };
    int rc = -1;

    if ((rc = ::poll(poll_fd, 1, duration.count())) < 0)
        throw std::system_error(errno, std::system_category());
    else if (rc == 0)
        throw Error::Timeout{};

    static const std::uint32_t value = 1;
    if (sizeof(value) != write(fds[write_fd], std::addressof(value), sizeof(value)))
        throw std::system_error(errno, std::system_category());
}

std::uint32_t core::testing::CrossProcessSync::wait_for_signal_ready_for(const std::chrono::milliseconds& duration)
{
    static const short empty_revents = 0;
    pollfd poll_fd[1] = { { fds[read_fd], POLLIN, empty_revents } };
    int rc = -1;

    if ((rc = ::poll(poll_fd, 1, duration.count())) < 0)
        throw std::system_error(errno, std::system_category());
    else if (rc == 0)
        throw Error::Timeout{};

    std::uint32_t value = 0;
    if (sizeof(value) != read(fds[read_fd], std::addressof(value), sizeof(value)))
        throw std::system_error(errno, std::system_category());

    if (value != 1)
        throw std::system_error(errno, std::system_category());

    counter += value;

    return counter;
}
