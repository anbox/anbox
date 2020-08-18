add_subdirectory(process-cpp-minimal)
add_subdirectory(android-emugl)
add_subdirectory(xdg)
add_subdirectory(backward-cpp)

set(BUILD_TESTING OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error")
add_subdirectory(cpu_features)
