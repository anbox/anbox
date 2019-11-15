project(anbox C CXX)
cmake_minimum_required(VERSION 2.8.2)

include(CTest)
include(GNUInstallDirs)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to RelWithDebInfo")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wno-variadic-macros -Wextra -fPIC")

# By default, all symbols are visible in the library. We strip out things we don't
# want at link time, by running a version script (see unity-scopes.map and the
# setting of LINK_FLAGS below).
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default")

set(C_AND_CXX_WARNINGS "-pedantic -Wall -Wextra")

# Some additional warnings not included by the general flags set above.
set(EXTRA_C_WARNINGS "-Wcast-align -Wcast-qual -Wformat -Wredundant-decls -Wswitch-default")
set(EXTRA_CXX_WARNINGS "-Wnon-virtual-dtor -Wold-style-cast")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_AND_CXX_WARNINGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_C_WARNINGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ${C_AND_CXX_WARNINGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_C_WARNINGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXX_WARNINGS}")

# -fno-permissive causes warnings with clang, so we only enable it for gcc
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-permissive")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_lower)

if ("${cmake_build_type_lower}" STREQUAL "release" OR "${cmake_build_type_lower}" STREQUAL "relwithdebinfo")
  option(Werror "Treat warnings as errors" ON)
else()
  option(Werror "Treat warnings as errors" OFF)
endif()

if (${Werror})
    message(STATUS "Treat warnings as errors")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  if ("${cmake_build_type_lower}" STREQUAL "release" OR "${cmake_build_type_lower}" STREQUAL "relwithdebinfo")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=deprecated-declarations")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated-declarations")
  endif()
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Boost COMPONENTS filesystem log serialization system thread program_options)
find_package(PkgConfig)
find_package(Threads)
find_package(EGL REQUIRED)
find_package(GLESv2 REQUIRED)
find_package(Protobuf REQUIRED)

pkg_check_modules(SDL2 sdl2 REQUIRED)
pkg_check_modules(SDL2_IMAGE SDL2_image REQUIRED)
pkg_check_modules(DBUS dbus-1 REQUIRED)
pkg_check_modules(LXC lxc REQUIRED)
pkg_check_modules(PROPERTIES_CPP properties-cpp REQUIRED)
pkg_check_modules(LIBSYSTEMD libsystemd REQUIRED)

message(STATUS "LXC version: ${LXC_VERSION}")
if (${LXC_VERSION} VERSION_LESS 3.0)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_LXC2_SUPPORT")
  message(STATUS "Building with LXC 2.x compatibility support")
endif()

option(ENABLE_X11 "Enable X11 support" ON)
if (ENABLE_X11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DX11_SUPPORT")
endif()

option(ENABLE_WAYLAND "Enable wayland support" ON)
if (ENABLE_WAYLAND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWAYLAND_SUPPORT")
endif()

option(ENABLE_MIR "Enable mir support" OFF)
if (ENABLE_MIR)
  pkg_check_modules(MIRCLIENT mirclient REQUIRED)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMIR_SUPPORT")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEGL_NO_X11")

if((Protobuf_VERSION VERSION_GREATER "3.7") OR (Protobuf_VERSION VERSION_EQUAL "3.7"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_PROTOBUF_CALLBACK_HEADER")
endif()

#####################################################################
# Enable code coverage calculation with gcov/gcovr/lcov
# Usage:
#  * Switch build type to coverage (use ccmake or cmake-gui)
#  * Invoke make, make test, make coverage
#  * Find html report in subdir coveragereport
#  * Find xml report feasible for jenkins in coverage.xml
#####################################################################
IF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ftest-coverage -fprofile-arcs" )
ENDIF(CMAKE_BUILD_TYPE MATCHES [cC][oO][vV][eE][rR][aA][gG][eE])

find_package(GMock)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC")


if (NOT ANBOX_VERSION)
    exec_program("git" ${CMAKE_SOURCE_DIR} ARGS "rev-parse --short HEAD" OUTPUT_VARIABLE GIT_COMMIT_HASH)
    set(ANBOX_VERSION "local-${GIT_COMMIT_HASH}")
endif()
if (ANBOX_VERSION_SUFFIX)
    set(ANBOX_VERSION "${ANBOX_VERSION}-${ANBOX_VERSION_SUFFIX}")
endif()
set(ANBOX_RESOURCE_DIR "${CMAKE_INSTALL_DATADIR}/anbox")
set(ANBOX_RESOURCE_DIR_FULL "${CMAKE_INSTALL_FULL_DATADIR}/anbox")
set(ANBOX_STATEDIR_FULL "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/anbox")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/anbox/build/config.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/src/anbox/build/config.h)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(android)

if (NOT "${HOST_CMAKE_C_COMPILER}" STREQUAL "")
  message(STATUS "Host C compiler: ${HOST_CMAKE_C_COMPILER}")
  message(STATUS "Host C compiler: ${HOST_CMAKE_CXX_COMPILER}")
endif()

option(SNAP_CONFINEMENT "Enable snap confinement support" OFF)
if (SNAP_CONFINEMENT)
  message(STATUS "Building with support for snap confinement")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_SNAP_CONFINEMENT")
endif()

install(FILES data/ui/loading-screen.png DESTINATION ${ANBOX_RESOURCE_DIR}/ui)

# uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
