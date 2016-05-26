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

#ifndef CORE_POSIX_EXIT_H_
#define CORE_POSIX_EXIT_H_

#include <cstdlib>

namespace core
{
namespace posix
{
namespace exit
{
    /**
     * @brief The Status enum wrap's the posix exit status.
     */
    enum class Status
    {
        success = EXIT_SUCCESS,
        failure = EXIT_FAILURE
    };
}
}
}

#endif // CORE_POSIX_EXIT_H_
