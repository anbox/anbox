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
