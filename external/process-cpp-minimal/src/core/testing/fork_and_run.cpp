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

#include <core/testing/fork_and_run.h>

#include <core/posix/exit.h>
#include <core/posix/fork.h>
#include <core/posix/wait.h>

core::testing::ForkAndRunResult core::testing::operator|(
        core::testing::ForkAndRunResult lhs,
        core::testing::ForkAndRunResult rhs)
{
    return static_cast<core::testing::ForkAndRunResult>(
                static_cast<unsigned int> (lhs) | static_cast<unsigned int>(rhs));
}

core::testing::ForkAndRunResult core::testing::operator&(
        core::testing::ForkAndRunResult lhs,
        core::testing::ForkAndRunResult rhs)
{
    return static_cast<core::testing::ForkAndRunResult>(
                static_cast<unsigned int> (lhs) & static_cast<unsigned int>(rhs));
}

core::testing::ForkAndRunResult core::testing::fork_and_run(
        const std::function<core::posix::exit::Status()>& service,
        const std::function<core::posix::exit::Status()>& client)
{
    core::testing::ForkAndRunResult result = core::testing::ForkAndRunResult::empty;

    auto service_process = core::posix::fork(service, core::posix::StandardStream::empty);
    auto client_process = core::posix::fork(client, core::posix::StandardStream::empty);

    auto client_result = client_process.wait_for(core::posix::wait::Flags::untraced);

    switch (client_result.status)
    {
    case core::posix::wait::Result::Status::exited:
        if (client_result.detail.if_exited.status == core::posix::exit::Status::failure)
            result = result | core::testing::ForkAndRunResult::client_failed;
        break;
    default:
        result = result | core::testing::ForkAndRunResult::client_failed;
        break;
    }

    service_process.send_signal_or_throw(core::posix::Signal::sig_term);
    auto service_result = service_process.wait_for(core::posix::wait::Flags::untraced);

    switch (service_result.status)
    {
    case core::posix::wait::Result::Status::exited:
        if (service_result.detail.if_exited.status == core::posix::exit::Status::failure)
            result = result | core::testing::ForkAndRunResult::service_failed;
        break;
    default:
        result = result | core::testing::ForkAndRunResult::service_failed;
        break;
    }

    return result;
}
