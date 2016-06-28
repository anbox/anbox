include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src)

set(ANBOXD_SOURCES
    ${CMAKE_SOURCE_DIR}/src/anbox/bridge/message_processor.cpp
    ${CMAKE_SOURCE_DIR}/src/anbox/bridge/pending_call_cache.cpp
    ${CMAKE_SOURCE_DIR}/src/anbox/common/fd.cpp
    service/server.cpp
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
