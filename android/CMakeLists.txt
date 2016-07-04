include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src)

set(ANBOXD_SOURCES
    ${CMAKE_SOURCE_DIR}/src/anbox/bridge/message_processor.cpp
    ${CMAKE_SOURCE_DIR}/src/anbox/bridge/pending_call_cache.cpp
    ${CMAKE_SOURCE_DIR}/src/anbox/common/fd.cpp
    service/platform_service.cpp
    service/platform_service_interface.cpp
    service/platform_api_stub.cpp
    service/android_api_skeleton.cpp
    service/message_processor.cpp
    service/local_socket_connection.cpp
    service/host_connector.cpp
    service/daemon.cpp
    service/main.cpp)

add_executable(anboxd ${ANBOXD_SOURCES})
target_link_libraries(anboxd
    pthread
    process-cpp
    anbox-protobuf)
# As we're adding Android specific bits in this project we can't
# build this safely within default build anymore. We keep this
# for easy integration into used IDEs.
set_target_properties(anboxd
    PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)

set(ANBOX_SHARED_COMPOSITOR
    shared_compositor/main.cpp
    shared_compositor/surface_composer.cpp)
add_executable(anbox_shared_compositor ${ANBOX_SHARED_COMPOSITOR})
set_target_properties(anbox_shared_compositor
    PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)

set(TEST_PLATFORM_SERVICE
    service/test_platform_service.cpp)

add_executable(test_platform_service ${TEST_PLATFORM_SERVICE})
# As we're adding Android specific bits in this project we can't
# build this safely within default build anymore. We keep this
# for easy integration into used IDEs.
set_target_properties(test_platform_service
    PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
