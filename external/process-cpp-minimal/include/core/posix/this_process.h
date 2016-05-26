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

#ifndef CORE_POSIX_THIS_PROCESS_H_
#define CORE_POSIX_THIS_PROCESS_H_

#include <core/posix/visibility.h>

#include <functional>
#include <iosfwd>
#include <string>
#include <system_error>

namespace core
{
namespace posix
{
class Process;
namespace this_process
{
namespace env
{
/**
 * @brief for_each invokes a functor for every key-value pair in the environment.
 * @param [in] functor Invoked for every key-value pair.
 */
CORE_POSIX_DLL_PUBLIC void for_each(
        const std::function<void(const std::string&, const std::string&)>& functor) noexcept(true);

/**
 * @brief get queries the value of an environment variable.
 * @throw std::runtime_error if there is no variable with the given key defined in the env.
 * @param [in] key Name of the variable to query the value for.
 * @return Contents of the variable.
 */
CORE_POSIX_DLL_PUBLIC std::string get_or_throw(const std::string& key);

/**
 * @brief get queries the value of an environment variable.
 * @param [in] key Name of the variable to query the value for.
 * @param [in] default_value Default value to return when key is not present in the environment.
 * @return Contents of the variable or an empty string if the variable is not defined.
 */
CORE_POSIX_DLL_PUBLIC std::string get(
        const std::string& key,
        const std::string& default_value = std::string()) noexcept(true);

/**
 * @brief unset_or_throw removes the variable with name key from the environment.
 * @throw std::system_error in case of errors.
 * @param [in] key Name of the variable to unset.
 */
CORE_POSIX_DLL_PUBLIC void unset_or_throw(const std::string& key);

/**
 * @brief unset removes the variable with name key from the environment.
 * @return false in case of errors, true otherwise.
 * @param [in] key Name of the variable to unset.
 * @param [out] se Receives error details if unset returns false.
 */
CORE_POSIX_DLL_PUBLIC bool unset(const std::string& key,
                            std::error_code& se) noexcept(true);

/**
 * @brief set_or_throw will adjust the contents of the variable identified by key to the provided value.
 * @throw std::system_error in case of errors.
 * @param [in] key Name of the variable to set the value for.
 * @param [in] value New contents of the variable.
 */
CORE_POSIX_DLL_PUBLIC void set_or_throw(const std::string& key,
                                   const std::string& value);
/**
 * @brief set will adjust the contents of the variable identified by key to the provided value.
 * @return false in case of errors, true otherwise.
 * @param [in] key Name of the variable to set the value for.
 * @param [in] value New contents of the variable.
 * @param [out] se Receives the details in case of errors.
 */
CORE_POSIX_DLL_PUBLIC bool set(const std::string &key,
                          const std::string &value,
                          std::error_code& se) noexcept(true);
}

/**
  * @brief Returns a Process instance corresponding to this process.
  */
CORE_POSIX_DLL_PUBLIC Process instance() noexcept(true);

/**
 * @brief Query the parent of the process.
 * @return The parent of the process.
 */
CORE_POSIX_DLL_PUBLIC Process parent() noexcept(true);

/**
 * @brief Access this process's stdin.
 */
CORE_POSIX_DLL_PUBLIC std::istream& cin() noexcept(true);

/**
 * @brief Access this process's stdout.
 */
CORE_POSIX_DLL_PUBLIC std::ostream& cout() noexcept(true);

/**
 * @brief Access this process's stderr.
 */
CORE_POSIX_DLL_PUBLIC std::ostream& cerr() noexcept(true);
}
}
}

#endif // CORE_POSIX_THIS_PROCESS_H_
