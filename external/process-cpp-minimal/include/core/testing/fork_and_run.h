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
#ifndef CORE_TESTING_FORK_AND_RUN_H_
#define CORE_TESTING_FORK_AND_RUN_H_

#include <core/posix/exit.h>
#include <core/posix/fork.h>
#include <core/posix/visibility.h>

#include <functional>

namespace core
{
namespace testing
{
/**
 * @brief The ForkAndRunResult enum models the different failure modes of fork_and_run.
 */
enum class ForkAndRunResult
{
    empty = 0, ///< Special value indicating no bit being set.
    client_failed = 1 << 0, ///< The client failed.
    service_failed = 1 << 1 ///< The service failed.
};

CORE_POSIX_DLL_PUBLIC ForkAndRunResult operator|(ForkAndRunResult lhs, ForkAndRunResult rhs);
CORE_POSIX_DLL_PUBLIC ForkAndRunResult operator&(ForkAndRunResult lhs, ForkAndRunResult rhs);

/**
 * @brief Forks two processes for both the service and the client.
 *
 * The function does the following:
 *   - Forks a process for the service and runs the respective closure.
 *   - Forks a process for the client and runs the respective closure.
 *   - After the client has finished, the service is signalled with sigterm.
 *
 * @throw std::system_error if an error occured during process interaction.
 * @throw std::runtime_error for signalling all other error conditions.
 * @param [in] service The service to be executed in a child process.
 * @param [in] client The client to be executed in a child process.
 * @return ForkAndRunResult indicating if either of service or client failed.
 */
CORE_POSIX_DLL_PUBLIC ForkAndRunResult fork_and_run(
        const std::function<core::posix::exit::Status()>& service,
        const std::function<core::posix::exit::Status()>& client);
}
}

/**
 * Test definition macro which runs a TEST in a forked process.
 * Note that you can only use EXPECT_*, not
 * ASSERT_*!
 *
 * Usage:
 * TESTP(test_suite, test_name, {
 *      test code ...
 *      EXPECT_* ...
 * })
 */
#define TESTP(test_suite, test_name, CODE)                                             \
    TEST(test_suite, test_name) {                                                       \
        auto test = [&]() {                                                             \
            CODE                                                                        \
            return HasFailure() ? core::posix::exit::Status::failure                    \
                            : core::posix::exit::Status::success;                       \
        };                                                                              \
        auto child = core::posix::fork(                                                 \
            test,                                                                       \
            core::posix::StandardStream::empty);                                        \
        auto result = child.wait_for(core::posix::wait::Flags::untraced);               \
        EXPECT_EQ(core::posix::wait::Result::Status::exited, result.status);            \
        EXPECT_EQ(core::posix::exit::Status::success, result.detail.if_exited.status);  \
    } \

/**
 * Test definition macro which runs a TEST_F in a forked process.
 * Note that you can only use EXPECT_*, not ASSERT_*!
 *
 * Usage:
 * TESTP_F(FixtureName, TestName, {
 *    ... test code ...
 *    EXPECT_* ...
 *  })
 */
#define TESTP_F(test_fixture, test_name, CODE)                                          \
    TEST_F(test_fixture, test_name) {                                                   \
        auto test = [&]() {                                                             \
            CODE                                                                        \
            return HasFailure() ? core::posix::exit::Status::failure                    \
                            : core::posix::exit::Status::success;                       \
        };                                                                              \
        auto child = core::posix::fork(                                                 \
            test,                                                                       \
            core::posix::StandardStream::empty);                                        \
        auto result = child.wait_for(core::posix::wait::Flags::untraced);               \
        EXPECT_EQ(core::posix::wait::Result::Status::exited, result.status);            \
        EXPECT_EQ(core::posix::exit::Status::success, result.detail.if_exited.status);  \
    } \

#endif // CORE_TESTING_FORK_AND_RUN_H_
