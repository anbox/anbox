set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare")

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/include
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/external/glm
)

macro(ANBOX_ADD_TEST test_name src)
  add_executable(
    ${test_name}
    ${src}
  )

  target_link_libraries(
    ${test_name}

    anbox-core

    ${GMOCK_LIBRARIES}
    ${GTEST_BOTH_LIBRARIES}

    ${ARGN}

    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
  )

  add_test(${test_name} ${CMAKE_CURRENT_BINARY_DIR}/${test_name} --gtest_filter=*-*requires*)
endmacro(ANBOX_ADD_TEST)

add_subdirectory(anbox)
