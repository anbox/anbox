# build google-test as subproject

include(ExternalProject)
include(FindPackageHandleStandardArgs)

#gtest
set(GTEST_INSTALL_DIR ${CMAKE_SOURCE_DIR}/external/googletest/googletest/include)

find_path(GTEST_INCLUDE_DIR gtest/gtest.h
            HINTS ${GTEST_INSTALL_DIR})

#gmock
find_path(GMOCK_INSTALL_DIR googlemock/CMakeLists.txt
          HINTS ${CMAKE_SOURCE_DIR}/external/googletest/)
if(${GMOCK_INSTALL_DIR} STREQUAL "GMOCK_INSTALL_DIR-NOTFOUND")
    message(FATAL_ERROR "google-mock package not found. Did you check out submodules?")
endif()

find_path(GMOCK_INCLUDE_DIR gmock/gmock.h 
          HINTS ${GMOCK_INSTALL_DIR}/include)

set(GOOGLE_TEST_SUBPROJECT_BIN_DIR ${CMAKE_BINARY_DIR}/googletest_bin)

set(GMOCK_PREFIX googlemock)
set(GMOCK_BINARY_DIR ${GOOGLE_TEST_SUBPROJECT_BIN_DIR}/${GMOCK_PREFIX})
set(GTEST_BINARY_DIR ${GOOGLE_TEST_SUBPROJECT_BIN_DIR}/${GMOCK_PREFIX}/gtest)

set(GTEST_CMAKE_ARGS "")

ExternalProject_Add(
    GMock
    #where to build in source tree
    PREFIX ${GMOCK_PREFIX}
    #where the source is external to the project
    SOURCE_DIR ${GMOCK_INSTALL_DIR}
    #forward the compilers to the subproject so cross-arch builds work
    CMAKE_ARGS ${GTEST_CMAKE_ARGS}
    BINARY_DIR ${GOOGLE_TEST_SUBPROJECT_BIN_DIR}#

    #we don't need to install, so skip
    INSTALL_COMMAND ""
)

set(GMOCK_LIBRARY ${GMOCK_BINARY_DIR}/libgmock.a)
set(GMOCK_MAIN_LIBRARY ${GMOCK_BINARY_DIR}/libgmock_main.a)
set(GMOCK_BOTH_LIBRARIES ${GMOCK_LIBRARY} ${GMOCK_MAIN_LIBRARY})
set(GTEST_LIBRARY ${GTEST_BINARY_DIR}/libgtest.a)
set(GTEST_MAIN_LIBRARY ${GTEST_BINARY_DIR}/libgtest_main.a)
set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARY}  ${GTEST_MAIN_LIBRARY})
set(GTEST_ALL_LIBRARIES ${GTEST_BOTH_LIBRARIES} ${GMOCK_BOTH_LIBRARIES})

find_package_handle_standard_args(GTest  DEFAULT_MSG
                                    GMOCK_INCLUDE_DIR
                                    GTEST_INCLUDE_DIR)
