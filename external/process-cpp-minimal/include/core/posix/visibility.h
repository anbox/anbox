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

#ifndef CORE_POSIX_VISIBILITY_H_
#define CORE_POSIX_VISIBILITY_H_

#if __GNUC__ >= 4
#define CORE_POSIX_DLL_PUBLIC __attribute__ ((visibility ("default")))
#define CORE_POSIX_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define CORE_POSIX_DLL_PUBLIC
#define CORE_POSIX_DLL_LOCAL
#endif

#endif // CORE_POSIX_VISIBILITY_H_
