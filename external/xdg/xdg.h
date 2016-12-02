// Copyright (C) 2015 Thomas Voß <thomas.voss.bochum@gmail.com>
//
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef XDG_H_
#define XDG_H_

#include <boost/filesystem.hpp>

#include <string>
#include <vector>

namespace xdg
{
// NotCopyable deletes the copy c'tor and the assignment operator.
struct NotCopyable
{
    NotCopyable() = default;
    NotCopyable(const NotCopyable&) = delete;
    virtual ~NotCopyable() = default;
    NotCopyable& operator=(const NotCopyable&) = delete;
};

// NotMoveable deletes the move c'tor and the move assignment operator.
struct NotMoveable
{
    NotMoveable() = default;
    NotMoveable(NotMoveable&&) = delete;
    virtual ~NotMoveable() = default;
    NotMoveable& operator=(NotMoveable&&) = delete;
};

// Data provides functions to query the XDG_DATA_* entries.
class Data : NotCopyable, NotMoveable
{
public:
    // home returns the base directory relative to which user specific
    // data files should be stored.
    virtual boost::filesystem::path home() const;
    // dirs returns the preference-ordered set of base directories to
    // search for data files in addition to the $XDG_DATA_HOME base
    // directory.
    virtual std::vector<boost::filesystem::path> dirs() const;
};

// Config provides functions to query the XDG_CONFIG_* entries.
class Config : NotCopyable, NotMoveable
{
public:
    // home returns the base directory relative to which user specific
    // configuration files should be stored.
    virtual boost::filesystem::path home() const;
    // dirs returns the preference-ordered set of base directories to
    // search for configuration files in addition to the
    // $XDG_CONFIG_HOME base directory.
    virtual std::vector<boost::filesystem::path> dirs() const;
};

// Cache provides functions to query the XDG_CACHE_HOME entry.
class Cache : NotCopyable, NotMoveable
{
public:
    // home returns the base directory relative to which user specific
    // non-essential data files should be stored.
    virtual boost::filesystem::path home() const;
};

// Runtime provides functions to query the XDG_RUNTIME_DIR entry.
class Runtime : NotCopyable, NotMoveable
{
public:
    // home returns the base directory relative to which user-specific
    // non-essential runtime files and other file objects (such as
    // sockets, named pipes, ...) should be stored.
    virtual boost::filesystem::path dir() const;
};

// A BaseDirSpecification implements the XDG base dir specification:
//   http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
class BaseDirSpecification : NotCopyable, NotMoveable
{
public:
    // create returns an Implementation of BaseDirSpecification.
    static std::shared_ptr<BaseDirSpecification> create();

    // data returns an immutable Data instance.
    virtual const Data& data() const = 0;
    // config returns an immutable Config instance.
    virtual const Config& config() const = 0;
    // cache returns an immutable Cache instance.
    virtual const Cache& cache() const = 0;
    // runtime returns an immutable Runtime instance.
    virtual const Runtime& runtime() const = 0;
protected:
    BaseDirSpecification() = default;
};

// data returns an immutable reference to a Data instance.
const Data& data();
// config returns an immutable reference to a Config instance.
const Config& config();
// cache returns an immutable reference to a Cache instance.
const Cache& cache();
// runtime returns an immutable reference to a Runtime instance.
const Runtime& runtime();
}

#endif // XDG_H_
