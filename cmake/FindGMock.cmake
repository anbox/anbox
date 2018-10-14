# Copyright (C) 2014 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Build with system gmock and embedded gtest
#
# Usage:
#
# find_package(GMock)
#
# ...
#
# target_link_libraries(
#   my-target
#   ${GTEST_BOTH_LIBRARIES}
# )
#
# NOTE: Due to the way this package finder is implemented, do not attempt
# to find the GMock package more than once.

find_package(Threads)

if (EXISTS "/usr/src/googletest")
    # As of version 1.8.0
    set(GMOCK_SOURCE_DIR "/usr/src/googletest/googlemock" CACHE PATH "gmock source directory")
    set(GMOCK_INCLUDE_DIRS "${GMOCK_SOURCE_DIR}/include" CACHE PATH "gmock source include directory")
    set(GTEST_INCLUDE_DIRS "/usr/src/googletest/googletest/include" CACHE PATH "gtest source include directory")
else()
    set(GMOCK_SOURCE_DIR "/usr/src/gmock" CACHE PATH "gmock source directory")
    set(GMOCK_INCLUDE_DIRS "/usr/include" CACHE PATH "gmock source include directory")
    set(GTEST_INCLUDE_DIRS "/usr/include" CACHE PATH "gtest source include directory")
endif()

# We add -g so we get debug info for the gtest stack frames with gdb.
# The warnings are suppressed so we get a noise-free build for gtest and gmock if the caller
# has these warnings enabled.
set(findgmock_cxx_flags "${CMAKE_CXX_FLAGS} -g -Wno-old-style-cast -Wno-missing-field-initializers -Wno-ctor-dtor-privacy -Wno-switch-default")

set(findgmock_bin_dir "${CMAKE_CURRENT_BINARY_DIR}/gmock")
set(findgmock_gtest_lib "${findgmock_bin_dir}/gtest/libgtest.a")
set(findgmock_gtest_main_lib "${findgmock_bin_dir}/gtest/libgtest_main.a")
set(findgmock_gmock_lib "${findgmock_bin_dir}/libgmock.a")
set(findgmock_gmock_main_lib "${findgmock_bin_dir}/libgmock_main.a")

include(ExternalProject)
ExternalProject_Add(GMock SOURCE_DIR "${GMOCK_SOURCE_DIR}"
                          BINARY_DIR "${findgmock_bin_dir}"
                          BUILD_BYPRODUCTS "${findgmock_gtest_lib}"
                                           "${findgmock_gtest_main_lib}"
                                           "${findgmock_gmock_lib}"
                                           "${findgmock_gmock_main_lib}"
                          INSTALL_COMMAND ""
                          CMAKE_ARGS "-DCMAKE_CXX_FLAGS=${findgmock_cxx_flags}")

add_library(gtest INTERFACE)
target_include_directories(gtest INTERFACE ${GTEST_INCLUDE_DIRS})
target_link_libraries(gtest INTERFACE ${findgmock_gtest_lib} ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(gtest GMock)

add_library(gtest_main INTERFACE)
target_include_directories(gtest_main INTERFACE ${GTEST_INCLUDE_DIRS})
target_link_libraries(gtest_main INTERFACE ${findgmock_gtest_main_lib} gtest)

add_library(gmock INTERFACE)
target_include_directories(gmock INTERFACE ${GMOCK_INCLUDE_DIRS})
target_link_libraries(gmock INTERFACE ${findgmock_gmock_lib} gtest)

add_library(gmock_main INTERFACE)
target_include_directories(gmock_main INTERFACE ${GMOCK_INCLUDE_DIRS})
target_link_libraries(gmock_main INTERFACE ${findgmock_gmock_main_lib} gmock gtest_main)

set(GTEST_LIBRARIES gtest)
set(GTEST_MAIN_LIBRARIES gtest_main)
set(GMOCK_LIBRARIES gmock gmock_main)
set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})

unset(findgmock_cxx_flags)
unset(findgmock_bin_dir)
unset(findgmock_gtest_lib)
unset(findgmock_gtest_main_lib)
unset(findgmock_gmock_lib)
unset(findgmock_gmock_main_lib)
