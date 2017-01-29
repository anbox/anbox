set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

set(AC_NDK_PATH $ENV{AC_NDK_PATH} CACHE STRING "path of mir android bundle")

if (NOT DEFINED AC_TARGET_MACHINE)
  set(AC_TARGET_MACHINE $ENV{AC_TARGET_MACHINE} CACHE STRING "target machine")
endif()
if (NOT DEFINED AC_GCC_VARIANT)
  set(AC_GCC_VARIANT $ENV{AC_GCC_VARIANT} CACHE STRING "gcc variant required")
endif()

set(HOST_CMAKE_C_COMPILER /usr/bin/cc)
set(HOST_CMAKE_CXX_COMPILER /usr/bin/c++)

set(CMAKE_C_COMPILER   /usr/bin/${AC_TARGET_MACHINE}-gcc${AC_GCC_VARIANT})
set(CMAKE_CXX_COMPILER /usr/bin/${AC_TARGET_MACHINE}-g++${AC_GCC_VARIANT})

# where to look to find dependencies in the target environment
set(CMAKE_FIND_ROOT_PATH  "${AC_NDK_PATH}")

#treat the chroot's includes as system includes
include_directories(SYSTEM "${AC_NDK_PATH}/usr/include" "${AC_NDK_PATH}/usr/include/${AC_TARGET_MACHINE}")
list(APPEND CMAKE_SYSTEM_INCLUDE_PATH "${AC_NDK_PATH}/usr/include" "${AC_NDK_PATH}/usr/include/${AC_TARGET_MACHINE}" )

# Add the chroot libraries as system libraries
list(APPEND CMAKE_SYSTEM_LIBRARY_PATH
  "${AC_NDK_PATH}/lib"
  "${AC_NDK_PATH}/lib/${AC_TARGET_MACHINE}"
  "${AC_NDK_PATH}/usr/lib"
  "${AC_NDK_PATH}/usr/lib/${AC_TARGET_MACHINE}"
)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_EXECUTABLE_RUNTIME_C_FLAG "-Wl,-rpath-link,")
set(CMAKE_EXECUTABLE_RUNTIME_CXX_FLAG "-Wl,-rpath-link,")
set(CMAKE_INSTALL_RPATH "${AC_NDK_PATH}/lib:${AC_NDK_PATH}/lib/${AC_TARGET_MACHINE}:${AC_NDK_PATH}/usr/lib:${AC_NDK_PATH}/usr/lib/${AC_TARGET_MACHINE}")

set(ENV{PKG_CONFIG_PATH} "${AC_NDK_PATH}/usr/lib/pkgconfig:${AC_NDK_PATH}/usr/lib/${AC_TARGET_MACHINE}/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "${AC_NDK_PATH}")

#use only the cross compile system
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
