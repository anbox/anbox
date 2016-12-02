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

#include <xdg.h>

#include <boost/test/unit_test.hpp>

#include <cstdlib>
#include <iostream>

BOOST_AUTO_TEST_CASE(XdgDataHomeThrowsForRelativeDirectoryFromEnv)
{
    ::setenv("XDG_DATA_HOME", "tmp", 1);
    BOOST_CHECK_THROW(xdg::BaseDirSpecification::create()->data().home(), std::runtime_error);
    BOOST_CHECK_THROW(xdg::data().home(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(XdgDataHomeReturnsDefaultValueForEmptyEnv)
{
    ::setenv("HOME", "/tmp", 1);
    ::setenv("XDG_DATA_HOME", "", 1);
    BOOST_CHECK_EQUAL("/tmp/.local/share", xdg::BaseDirSpecification::create()->data().home());
    BOOST_CHECK_EQUAL("/tmp/.local/share", xdg::data().home());
}

BOOST_AUTO_TEST_CASE(XdgDataDirsCorrectlyTokenizesEnv)
{
    ::setenv("XDG_DATA_DIRS", "/tmp:/tmp", 1);
    BOOST_CHECK(2 == xdg::BaseDirSpecification::create()->data().dirs().size());
    BOOST_CHECK(2 == xdg::data().dirs().size());
}

BOOST_AUTO_TEST_CASE(XdgDataDirsThrowsForRelativeDirectoryFromEnv)
{
    ::setenv("XDG_DATA_DIRS", "/tmp:tmp", 1);
    BOOST_CHECK_THROW(xdg::BaseDirSpecification::create()->data().dirs(), std::runtime_error);
    BOOST_CHECK_THROW(xdg::data().dirs(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(XdgDataDirsReturnsDefaultValueForEmptyEnv)
{
    ::setenv("XDG_DATA_DIRS", "", 1);
    auto dirs = xdg::data().dirs();
    BOOST_CHECK_EQUAL("/usr/local/share", dirs[0]);
    BOOST_CHECK_EQUAL("/usr/share", dirs[1]);

    dirs = xdg::BaseDirSpecification::create()->data().dirs();
    BOOST_CHECK_EQUAL("/usr/local/share", dirs[0]);
    BOOST_CHECK_EQUAL("/usr/share", dirs[1]);
}

BOOST_AUTO_TEST_CASE(XdgConfigHomeThrowsForRelativeDirectoryFromEnv)
{
    ::setenv("XDG_CONFIG_HOME", "tmp", 1);
    BOOST_CHECK_THROW(xdg::BaseDirSpecification::create()->config().home(), std::runtime_error);
    BOOST_CHECK_THROW(xdg::config().home(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(XdgConfigHomeReturnsDefaultValueForEmptyEnv)
{
    ::setenv("HOME", "/tmp", 1);
    ::setenv("XDG_CONFIG_HOME", "", 1);
    BOOST_CHECK_EQUAL("/tmp/.config", xdg::BaseDirSpecification::create()->config().home());
    BOOST_CHECK_EQUAL("/tmp/.config", xdg::config().home());
}

BOOST_AUTO_TEST_CASE(XdgConfigDirsCorrectlyTokenizesEnv)
{
    ::setenv("XDG_CONFIG_DIRS", "/tmp:/tmp", 1);
    BOOST_CHECK(2 == xdg::BaseDirSpecification::create()->config().dirs().size());
    BOOST_CHECK(2 == xdg::config().dirs().size());
}

BOOST_AUTO_TEST_CASE(XdgConfigDirsThrowsForRelativeDirectoryFromEnv)
{
    ::setenv("XDG_CONFIG_DIRS", "/tmp:tmp", 1);
    BOOST_CHECK_THROW(xdg::BaseDirSpecification::create()->config().dirs(), std::runtime_error);
    BOOST_CHECK_THROW(xdg::config().dirs(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(XdgConfigDirsReturnsDefaultValueForEmptyEnv)
{
    ::setenv("XDG_CONFIG_DIRS", "", 1);
    auto dirs = xdg::config().dirs();
    BOOST_CHECK_EQUAL("/etc/xdg", dirs[0]);
    dirs = xdg::BaseDirSpecification::create()->config().dirs();
    BOOST_CHECK_EQUAL("/etc/xdg", dirs[0]);
}

BOOST_AUTO_TEST_CASE(XdgCacheHomeThrowsForRelativeDirectoryFromEnv)
{
    ::setenv("XDG_CACHE_HOME", "tmp", 1);
    BOOST_CHECK_THROW(xdg::BaseDirSpecification::create()->cache().home(), std::runtime_error);
    BOOST_CHECK_THROW(xdg::cache().home(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(XdgCacheHomeReturnsDefaultValueForEmptyEnv)
{
    ::setenv("HOME", "/tmp", 1);
    ::setenv("XDG_CACHE_HOME", "", 1);
    BOOST_CHECK_EQUAL("/tmp/.cache", xdg::BaseDirSpecification::create()->cache().home());
    BOOST_CHECK_EQUAL("/tmp/.cache", xdg::cache().home());
}

BOOST_AUTO_TEST_CASE(XdgRuntimeDirThrowsForRelativeDirectoryFromEnv)
{
    ::setenv("XDG_RUNTIME_DIR", "tmp", 1);
    BOOST_CHECK_THROW(xdg::BaseDirSpecification::create()->runtime().dir(), std::runtime_error);
    BOOST_CHECK_THROW(xdg::runtime().dir(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(XdgRuntimeDirThrowsForEmptyEnv)
{
    ::setenv("XDG_RUNTIME_DIR", "", 1);
    BOOST_CHECK_THROW(xdg::BaseDirSpecification::create()->runtime().dir(), std::runtime_error);
    BOOST_CHECK_THROW(xdg::runtime().dir(), std::runtime_error);
}

