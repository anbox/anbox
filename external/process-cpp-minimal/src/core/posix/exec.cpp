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

#include <core/posix/exec.h>
#include <core/posix/fork.h>
#include <core/posix/standard_stream.h>

#include <iostream>

#include <cstring>

#include <unistd.h>

namespace core
{
namespace posix
{
ChildProcess exec(const std::string& fn,
                  const std::vector<std::string>& argv,
                  const std::map<std::string, std::string>& env,
                  const StandardStream& flags)
{
    std::function<void()> null_function = [](){};
    return exec(fn, argv, env, flags, null_function);
}

ChildProcess exec(const std::string& fn,
                  const std::vector<std::string>& argv,
                  const std::map<std::string, std::string>& env,
                  const StandardStream& flags,
                  const std::function<void()>& child_setup)
{
    return posix::fork([fn, argv, env, child_setup]()
    {
        char** it; char** pargv; char** penv;
        it = pargv = new char*[argv.size()+2];
        *it = ::strdup(fn.c_str());
        it++;
        for (auto element : argv)
        {
            *it = ::strdup(element.c_str());
            it++;
        }
        *it = nullptr;

        it = penv = new char*[env.size()+1];
        for (auto pair : env)
        {
            *it = ::strdup((pair.first + "=" + pair.second).c_str());
            it++;
        }
        *it = nullptr;

        child_setup();
        return static_cast<posix::exit::Status>(execve(fn.c_str(), pargv, penv));
    }, flags);
}

}
}
