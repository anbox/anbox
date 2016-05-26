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

#include <core/posix/this_process.h>
#include <core/posix/process.h>

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <mutex>
#include <sstream>
#include <vector>

#include <cerrno>
#include <cstdlib>

#if defined(_GNU_SOURCE)
#include <unistd.h>
#else
extern char** environ;
#endif

namespace core
{
namespace posix
{
namespace this_process
{
namespace env
{
namespace
{
std::mutex& env_guard()
{
    static std::mutex m;
    return m;
}
}

void for_each(const std::function<void(const std::string&, const std::string&)>& functor) noexcept(true)
{
    std::lock_guard<std::mutex> lg(env_guard());
    auto it = ::environ;
    while (it != nullptr && *it != nullptr)
    {
        std::string line(*it);        
        functor(line.substr(0,line.find_first_of('=')),
                line.substr(line.find_first_of('=')+1));
        ++it;
    }
}

std::string get_or_throw(const std::string& key)
{
    std::lock_guard<std::mutex> lg(env_guard());

    auto result = ::getenv(key.c_str());

    if (result == nullptr)
    {
        std::stringstream ss;
        ss << "Variable with name " << key << " is not defined in the environment";
        throw std::runtime_error(ss.str());
    }

    return std::string{result};
}

std::string get(const std::string& key,
                const std::string& default_value) noexcept(true)
{
    std::lock_guard<std::mutex> lg(env_guard());

    auto result = ::getenv(key.c_str());
    return std::string{result ? result : default_value};
}

void unset_or_throw(const std::string& key)
{
    std::lock_guard<std::mutex> lg(env_guard());

    auto rc = ::unsetenv(key.c_str());

    if (rc == -1)
        throw std::system_error(errno, std::system_category());
}

bool unset(const std::string& key,
           std::error_code& se) noexcept(true)
{
    std::lock_guard<std::mutex> lg(env_guard());

    auto rc = ::unsetenv(key.c_str());

    if (rc == -1)
    {
        se = std::error_code(errno, std::system_category());
        return false;
    }

    return true;
}

void set_or_throw(const std::string& key,
                  const std::string& value)
{
    std::lock_guard<std::mutex> lg(env_guard());

    static const int overwrite = 0;
    auto rc = ::setenv(key.c_str(), value.c_str(), overwrite);

    if (rc == -1)
        throw std::system_error(errno, std::system_category());
}

bool set(const std::string &key,
         const std::string &value,
         std::error_code& se) noexcept(true)
{
    std::lock_guard<std::mutex> lg(env_guard());

    static const int overwrite = 0;
    auto rc = ::setenv(key.c_str(), value.c_str(), overwrite);

    if (rc == -1)
    {
        se = std::error_code(errno, std::system_category());
        return false;
    }

    return true;
}
}

Process instance() noexcept(true)
{
    static const Process self{getpid()};
    return self;
}

Process parent() noexcept(true)
{
    return Process(getppid());
}

std::istream& cin() noexcept(true)
{
    return std::cin;
}

std::ostream& cout() noexcept(true)
{
    return std::cout;
}

std::ostream& cerr() noexcept(true)
{
    return std::cerr;
}
}
}
}
