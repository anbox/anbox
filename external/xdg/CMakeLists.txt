# We have to manually alter the cxx flags to have a working 
# travis-ci build. Its container-based infrastructure only features 
# a very old cmake that does not support the more current:
#   set_property(TARGET xdg_test PROPERTY CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Boost COMPONENTS filesystem system unit_test_framework)

include_directories(
    .
    ${Boost_INCLUDE_DIRS}
)

add_library(xdg STATIC xdg.cpp)
set_property(TARGET xdg PROPERTY CXX_STANDARD 11)
target_link_libraries(xdg ${Boost_LIBRARIES})

enable_testing()
add_definitions(-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN -DBOOST_TEST_MODULE=xdg)
add_executable(xdg_test xdg_test.cpp)
set_property(TARGET xdg_test PROPERTY CXX_STANDARD 11)
target_link_libraries(xdg_test xdg ${Boost_LIBRARIES})

add_test(xdg_test xdg_test)
