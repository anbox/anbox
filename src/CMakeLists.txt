include_directories(
  ${Boost_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GIO_INCLUDE_DIRS}
  ${GIO-UNIX_INCLUDE_DIRS}
  ${MIRCLIENT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/external/process-cpp-minimal/include
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/include
)

set(SOURCES
    anbox/logger.cpp
    anbox/utils.cpp
    anbox/cli.cpp
    anbox/runtime.cpp
    anbox/version.cpp
    anbox/daemon.cpp
    anbox/config.cpp
    anbox/not_reachable.cpp
    anbox/pid_persister.cpp
    anbox/namespace_attacher.cpp
    anbox/container.cpp
    anbox/container_connector.cpp
    anbox/input_channel.cpp

    anbox/common/fd.cpp
    anbox/common/fd_sets.h
    anbox/common/variable_length_array.h

    anbox/network/message_sender.h
    anbox/network/message_receiver.h
    anbox/network/message_processor.h
    anbox/network/connector.h
    anbox/network/connection_creator.h
    anbox/network/qemu_pipe_connection_creator.cpp
    anbox/network/published_socket_connector.cpp
    anbox/network/connections.h
    anbox/network/socket_connection.cpp
    anbox/network/socket_messenger.cpp
    anbox/network/delegate_message_processor.h
    anbox/network/delegate_connection_creator.h

    anbox/graphics/opengles_message_processor.cpp
    anbox/graphics/mir_display_connection.cpp
    anbox/graphics/mir_window.cpp
    anbox/graphics/mir_native_window_creator.cpp
    anbox/graphics/gl_renderer_server.cpp

    anbox/input/manager.cpp
    anbox/input/device.cpp

    anbox/support/null_message_processor.cpp
    anbox/support/qemud_message_processor.cpp
    anbox/support/boot_properties_message_processor.cpp
    anbox/support/hwcontrol_message_processor.cpp
    anbox/support/sensors_message_processor.cpp
    anbox/support/camera_message_processor.cpp
    anbox/support/fingerprint_message_processor.cpp

    anbox/cmds/version.cpp
    anbox/cmds/run.cpp
    anbox/cmds/shell.cpp

    anbox/do_not_copy_or_move.h
    anbox/optional.h
    anbox/defer_action.h)

add_library(anbox-core ${SOURCES})
target_link_libraries(anbox-core
  ${Boost_LDFLAGS}
  ${Boost_LIBRARIES}
  ${MIRCLIENT_LDFLAGS}
  ${MIRCLIENT_LIBRARIES}
  pthread
  process-cpp
  OpenglRender)

add_executable(anbox main.cpp)
target_link_libraries(anbox
    anbox-core)

add_executable(anbox-container container_main.cpp)
target_link_libraries(anbox-container
    bwrap)

install(
  TARGETS anbox
  RUNTIME DESTINATION sbin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)
