# - Try to find GLESv2
# Once done this will define
#  GLESv2_FOUND - System has GLESv2
#  GLESv2_INCLUDE_DIRS - The GLESv2 include directories
#  GLESv2_LIBRARIES - The libraries needed to use GLESv2

find_package(PkgConfig)
pkg_check_modules(PC_GLESv2 QUIET glesv2)

find_path(GLESv2_INCLUDE_DIR GLES2/gl2.h
          HINTS ${PC_GLESv2_INCLUDEDIR} ${PC_GLESv2_INCLUDE_DIRS})

find_library(GLESv2_LIBRARY GLESv2
             HINTS ${PC_GLESv2_LIBDIR} ${PC_GLESv2_LIBRARY_DIRS})

set(GLESv2_LIBRARIES ${GLESv2_LIBRARY})
set(GLESv2_INCLUDE_DIRS ${GLESv2_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set GLESv2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(GLESv2  DEFAULT_MSG
                                  GLESv2_LIBRARY GLESv2_INCLUDE_DIR)

mark_as_advanced(GLESv2_INCLUDE_DIR GLESv2_LIBRARY)
