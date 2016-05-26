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

#ifndef CORE_POSIX_FORK_H_
#define CORE_POSIX_FORK_H_

#include <core/posix/child_process.h>
#include <core/posix/standard_stream.h>
#include <core/posix/visibility.h>

#include <functional>

namespace core
{
namespace posix
{
/**
 * @brief fork forks a new process and executes the provided main function in the newly forked process.
 * @throws std::system_error in case of errors.
 * @param [in] main The main function of the newly forked process.
 * @param [in] flags Specify which standard streams should be redirected to the parent process.
 * @return An instance of ChildProcess in case of success.
 */
CORE_POSIX_DLL_PUBLIC ChildProcess fork(const std::function<posix::exit::Status()>& main,
                                   const StandardStream& flags);

/**
 * @brief fork vforks a new process and executes the provided main function in the newly forked process.
 * @throws std::system_error in case of errors.
 * @param [in] main The main function of the newly forked process.
 * @param [in] flags Specify which standard streams should be redirected to the parent process.
 * @return An instance of ChildProcess in case of success.
 */
CORE_POSIX_DLL_PUBLIC ChildProcess vfork(const std::function<posix::exit::Status()>& main,
                   const StandardStream& flags);
}
}

#endif // CORE_POSIX_FORK_H_
