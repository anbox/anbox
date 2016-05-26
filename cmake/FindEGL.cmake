# - Try to find EGL
# Once done this will define
#  EGL_FOUND - System has EGL
#  EGL_INCLUDE_DIRS - The EGL include directories
#  EGL_LIBRARIES - The libraries needed to use EGL

find_package(PkgConfig)
pkg_check_modules(PC_EGL QUIET egl)

find_path(EGL_INCLUDE_DIR EGL/egl.h
          HINTS ${PC_EGL_INCLUDEDIR} ${PC_EGL_INCLUDE_DIRS})

find_library(EGL_LIBRARY EGL
             HINTS ${PC_EGL_LIBDIR} ${PC_EGL_LIBRARY_DIRS})

set(EGL_LIBRARIES ${EGL_LIBRARY})
set(EGL_INCLUDE_DIRS ${EGL_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EGL_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(EGL  DEFAULT_MSG
                                  EGL_LIBRARY EGL_INCLUDE_DIR)

mark_as_advanced(EGL_INCLUDE_DIR EGL_LIBRARY)
