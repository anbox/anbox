# Copyright © 2013 Canonical Ltd.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Thomas Voss <thomas.voss@canonical.com>

cmake_minimum_required(VERSION 2.8)

project(process-cpp)

find_package(Boost COMPONENTS iostreams system REQUIRED)
find_package(Threads REQUIRED)

include(GNUInstallDirs)

set(PROCESS_CPP_VERSION_MAJOR 2)
set(PROCESS_CPP_VERSION_MINOR 0)
set(PROCESS_CPP_VERSION_PATCH 0)

include(CTest)

include_directories(
  include/
  src/
  external/process-cpp-minimal
  ${Boost_INCLUDE_DIRS}
)

add_subdirectory(src)
