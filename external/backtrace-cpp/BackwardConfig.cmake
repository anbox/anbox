#
# BackwardMacros.cmake
# Copyright 2013 Google Inc. All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

###############################################################################
# OPTIONS
###############################################################################

set(STACK_WALKING_UNWIND TRUE CACHE BOOL
	"Use compiler's unwind API")
set(STACK_WALKING_BACKTRACE FALSE CACHE BOOL
	"Use backtrace from (e)glibc for stack walking")

set(STACK_DETAILS_AUTO_DETECT TRUE CACHE BOOL
	"Auto detect backward's stack details dependencies")

set(STACK_DETAILS_BACKTRACE_SYMBOL FALSE CACHE BOOL
	"Use backtrace from (e)glibc for symbols resolution")
set(STACK_DETAILS_DW FALSE CACHE BOOL
	"Use libdw to read debug info")
set(STACK_DETAILS_BFD FALSE CACHE BOOL
	"Use libbfd to read debug info")

###############################################################################
# CONFIGS
###############################################################################
if (${STACK_DETAILS_AUTO_DETECT})
	include(FindPackageHandleStandardArgs)

	# find libdw
	find_path(LIBDW_INCLUDE_DIR NAMES "elfutils/libdw.h" "elfutils/libdwfl.h")
	find_library(LIBDW_LIBRARY dw)
	set(LIBDW_INCLUDE_DIRS ${LIBDW_INCLUDE_DIR} )
	set(LIBDW_LIBRARIES ${LIBDW_LIBRARY} )
	find_package_handle_standard_args(libdw DEFAULT_MSG
		LIBDW_LIBRARY LIBDW_INCLUDE_DIR)
	mark_as_advanced(LIBDW_INCLUDE_DIR LIBDW_LIBRARY)

	# find libbfd
	find_path(LIBBFD_INCLUDE_DIR NAMES "bfd.h")
	find_path(LIBDL_INCLUDE_DIR NAMES "dlfcn.h")
	find_library(LIBBFD_LIBRARY bfd)
	find_library(LIBDL_LIBRARY dl)
	set(LIBBFD_INCLUDE_DIRS ${LIBBFD_INCLUDE_DIR} ${LIBDL_INCLUDE_DIR})
	set(LIBBFD_LIBRARIES ${LIBBFD_LIBRARY} ${LIBDL_LIBRARY})
	find_package_handle_standard_args(libbfd DEFAULT_MSG
		LIBBFD_LIBRARY LIBBFD_INCLUDE_DIR
		LIBDL_LIBRARY LIBDL_INCLUDE_DIR)
	mark_as_advanced(LIBBFD_INCLUDE_DIR LIBBFD_LIBRARY
		LIBDL_INCLUDE_DIR LIBDL_LIBRARY)

	if (LIBDW_FOUND)
		LIST(APPEND _BACKWARD_INCLUDE_DIRS ${LIBDW_INCLUDE_DIRS})
		LIST(APPEND BACKWARD_LIBRARIES ${LIBDW_LIBRARIES})
		set(STACK_DETAILS_DW TRUE)
		set(STACK_DETAILS_BFD FALSE)
		set(STACK_DETAILS_BACKTRACE_SYMBOL FALSE)
	elseif(LIBBFD_FOUND)
		LIST(APPEND _BACKWARD_INCLUDE_DIRS ${LIBBFD_INCLUDE_DIRS})
		LIST(APPEND BACKWARD_LIBRARIES ${LIBBFD_LIBRARIES})
		set(STACK_DETAILS_DW FALSE)
		set(STACK_DETAILS_BFD TRUE)
		set(STACK_DETAILS_BACKTRACE_SYMBOL FALSE)
	else()
		set(STACK_DETAILS_DW FALSE)
		set(STACK_DETAILS_BFD FALSE)
		set(STACK_DETAILS_BACKTRACE_SYMBOL TRUE)
	endif()
else()
	if (STACK_DETAILS_DW)
		LIST(APPEND BACKWARD_LIBRARIES dw)
	endif()

	if (STACK_DETAILS_BFD)
		LIST(APPEND BACKWARD_LIBRARIES bfd dl)
	endif()
endif()

macro(map_definitions var_prefix define_prefix)
	foreach(def ${ARGN})
		if (${${var_prefix}${def}})
			LIST(APPEND BACKWARD_DEFINITIONS "${define_prefix}${def}=1")
		else()
			LIST(APPEND BACKWARD_DEFINITIONS "${define_prefix}${def}=0")
		endif()
	endforeach()
endmacro()

if (NOT BACKWARD_DEFINITIONS)
	map_definitions("STACK_WALKING_" "BACKWARD_HAS_" UNWIND BACKTRACE)
	map_definitions("STACK_DETAILS_" "BACKWARD_HAS_" BACKTRACE_SYMBOL DW BFD)
endif()

foreach(def ${BACKWARD_DEFINITIONS})
	message(STATUS "${def}")
endforeach()

find_path(BACKWARD_INCLUDE_DIR backward.hpp PATHS ${CMAKE_CURRENT_LIST_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Backward
    REQUIRED_VARS
        BACKWARD_INCLUDE_DIR
        BACKWARD_LIBRARIES
)
list(APPEND _BACKWARD_INCLUDE_DIRS ${BACKWARD_INCLUDE_DIR})

macro(add_backward target)
	target_include_directories(${target} PRIVATE ${_BACKWARD_INCLUDE_DIRS})
	set_property(TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS ${BACKWARD_DEFINITIONS})
	set_property(TARGET ${target} APPEND PROPERTY LINK_LIBRARIES ${BACKWARD_LIBRARIES})
endmacro()
set(BACKWARD_INCLUDE_DIRS ${_BACKWARD_INCLUDE_DIRS} CACHE INTERNAL "_BACKWARD_INCLUDE_DIRS")
set(BACKWARD_DEFINITIONS ${BACKWARD_DEFINITIONS} CACHE INTERNAL "BACKWARD_DEFINITIONS")
set(BACKWARD_LIBRARIES ${BACKWARD_LIBRARIES} CACHE INTERNAL "BACKWARD_LIBRARIES")
mark_as_advanced(_BACKWARD_INCLUDE_DIRS BACKWARD_DEFINITIONS BACKWARD_LIBRARIES)

if (NOT TARGET Backward::Backward)
	add_library(Backward::Backward INTERFACE IMPORTED)
	set_target_properties(Backward::Backward PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${_BACKWARD_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${BACKWARD_LIBRARIES}"
        INTERFACE_COMPILE_DEFINITIONS "${BACKWARD_DEFINITIONS}"
    )
endif()