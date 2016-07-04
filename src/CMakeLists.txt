include_directories(
  ${Boost_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GIO_INCLUDE_DIRS}
  ${GIO-UNIX_INCLUDE_DIRS}
  ${MIRCLIENT_INCLUDE_DIRS}
  ${DBUS_CPP_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/external/process-cpp-minimal/include
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/include
)

set(PROTOBUF_SOURCES
    anbox/protobuf/anbox_bridge.proto)

protobuf_generate_cpp(
    GENERATED_PROTOBUF_SRCS GENERATED_PROTOBUF_HDRS
    ${PROTOBUF_SOURCES})

add_library(anbox-protobuf
    ${GENERATED_PROTOBUF_SRCS}
    ${GENERATED_PROTOBUF_HDRS}
    anbox/protobuf/google_protobuf_guard.cpp)
target_link_libraries(anbox-protobuf
    ${PROTOBUF_LITE_LIBRARIES})

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
    anbox/application_manager.h

    anbox/common/fd.cpp
    anbox/common/fd_sets.h
    anbox/common/variable_length_array.h
    anbox/common/wait_handle.cpp
    anbox/common/dispatcher.cpp

    anbox/network/message_sender.h
    anbox/network/message_receiver.h
    anbox/network/message_processor.h
    anbox/network/connector.h
    anbox/network/connection_creator.h
    anbox/network/published_socket_connector.cpp
    anbox/network/connections.h
    anbox/network/socket_connection.cpp
    anbox/network/socket_messenger.cpp
    anbox/network/delegate_message_processor.h
    anbox/network/delegate_connection_creator.h

    anbox/graphics/opengles_message_processor.cpp
    anbox/graphics/gl_renderer_server.cpp
    anbox/graphics/window_creator.cpp

    anbox/input/manager.cpp
    anbox/input/device.cpp

    anbox/qemu/pipe_connection_creator.cpp
    anbox/qemu/null_message_processor.cpp
    anbox/qemu/qemud_message_processor.cpp
    anbox/qemu/boot_properties_message_processor.cpp
    anbox/qemu/hwcontrol_message_processor.cpp
    anbox/qemu/sensors_message_processor.cpp
    anbox/qemu/camera_message_processor.cpp
    anbox/qemu/fingerprint_message_processor.cpp
    anbox/qemu/gsm_message_processor.cpp
    anbox/qemu/at_parser.cpp
    anbox/qemu/bootanimation_message_processor.cpp

    anbox/bridge/constants.h
    anbox/bridge/connection_creator.cpp
    anbox/bridge/message_processor.cpp
    anbox/bridge/template_message_processor.h
    anbox/bridge/pending_call_cache.cpp
    anbox/bridge/make_protobuf_object.h
    anbox/bridge/platform_message_processor.cpp
    anbox/bridge/platform_api_skeleton.cpp
    anbox/bridge/rpc_channel.cpp
    anbox/bridge/android_api_stub.cpp

    anbox/ubuntu/platform_api_skeleton.cpp
    anbox/ubuntu/mir_display_connection.cpp
    anbox/ubuntu/window_creator.cpp
    anbox/ubuntu/window.cpp

    anbox/dbus/interface.h
    anbox/dbus/skeleton/service.cpp
    anbox/dbus/skeleton/application_manager.cpp
    anbox/dbus/stub/application_manager.cpp

    anbox/cmds/version.cpp
    anbox/cmds/run.cpp
    anbox/cmds/shell.cpp
    anbox/cmds/install_app.cpp
    anbox/cmds/launch_app.cpp
    anbox/cmds/reset.cpp

    anbox/do_not_copy_or_move.h
    anbox/optional.h
    anbox/defer_action.h)

add_library(anbox-core ${SOURCES})
target_link_libraries(anbox-core
  ${Boost_LDFLAGS}
  ${Boost_LIBRARIES}
  ${MIRCLIENT_LDFLAGS}
  ${MIRCLIENT_LIBRARIES}
  ${DBUS_CPP_LIBRARIES}
  pthread
  process-cpp
  OpenglRender)

add_executable(anbox main.cpp)
target_link_libraries(anbox
    anbox-core
    anbox-protobuf)

add_executable(anbox-container container_main.cpp)
target_link_libraries(anbox-container
    bwrap)

install(
  TARGETS anbox
  RUNTIME DESTINATION sbin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)
