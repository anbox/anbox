include_directories(
  ${Boost_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GIO_INCLUDE_DIRS}
  ${GIO-UNIX_INCLUDE_DIRS}
  ${MIRCLIENT_INCLUDE_DIRS}
  ${DBUS_CPP_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
  ${SDL2_IMAGE_INCLUDE_DIRS}
  ${PROPERTIES_CPP_INCLUDE_DIRS}
  ${LXC_INCLUDE_DIRS}
  ${MIRCLIENT_INCLUDE_DIRS}
  ${LIBSYSTEMD_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/process-cpp-minimal/include
  ${CMAKE_SOURCE_DIR}/external/android-emugl/shared
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/include
  ${CMAKE_BINARY_DIR}/external/android-emugl/host/include
  ${CMAKE_SOURCE_DIR}/external/android-emugl/shared/OpenglCodecCommon
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/include/libOpenglRender
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs/GLESv1_dec
  ${CMAKE_BINARY_DIR}/external/android-emugl/host/libs/GLESv1_dec
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs/GLESv2_dec
  ${CMAKE_BINARY_DIR}/external/android-emugl/host/libs/GLESv2_dec
  ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs/renderControl_dec
  ${CMAKE_BINARY_DIR}/external/android-emugl/host/libs/renderControl_dec
  ${CMAKE_SOURCE_DIR}/external/glm
)

protobuf_generate_cpp(
    GENERATED_PROTOBUF_RPC_SRCS GENERATED_PROTOBUF_RPC_HDRS
    anbox/protobuf/anbox_rpc.proto)

protobuf_generate_cpp(
    GENERATED_PROTOBUF_BRIDGE_SRCS GENERATED_PROTOBUF_BRIDGE_HDRS
    anbox/protobuf/anbox_bridge.proto)

protobuf_generate_cpp(
    GENERATED_PROTOBUF_CONTAINER_SRCS GENERATED_PROTOBUF_CONTAINER_HDRS
    anbox/protobuf/anbox_container.proto)


add_library(anbox-protobuf
    STATIC
    ${GENERATED_PROTOBUF_BRIDGE_SRCS}
    ${GENERATED_PROTOBUF_BRIDGE_HDRS}
    ${GENERATED_PROTOBUF_RPC_SRCS}
    ${GENERATED_PROTOBUF_RPC_HDRS}
    ${GENERATED_PROTOBUF_CONTAINER_SRCS}
    ${GENERATED_PROTOBUF_CONTAINER_HDRS}
    anbox/protobuf/anbox_rpc.proto
    anbox/protobuf/anbox_bridge.proto
    anbox/protobuf/anbox_container.proto
    anbox/protobuf/google_protobuf_guard.cpp)
target_link_libraries(anbox-protobuf
    ${PROTOBUF_LITE_LIBRARIES})

set(SOURCES
    anbox/logger.cpp
    anbox/utils.cpp
    anbox/cli.cpp
    anbox/runtime.cpp
    anbox/daemon.cpp
    anbox/system_configuration.cpp
    anbox/not_reachable.cpp

    anbox/build/config.h.in

    anbox/android/intent.cpp
    anbox/android/ip_config_builder.cpp

    anbox/common/fd.cpp
    anbox/common/fd_sets.h
    anbox/common/variable_length_array.h
    anbox/common/wait_handle.cpp
    anbox/common/dispatcher.cpp
    anbox/common/small_vector.h
    anbox/common/type_traits.h
    anbox/common/message_channel.cpp
    anbox/common/scope_ptr.h
    anbox/common/loop_device.cpp
    anbox/common/loop_device_allocator.cpp
    anbox/common/mount_entry.cpp
    anbox/common/binary_writer.cpp

    anbox/testing/gtest_utils.h

    anbox/container/service.cpp
    anbox/container/client.cpp
    anbox/container/configuration.h
    anbox/container/container.cpp
    anbox/container/lxc_container.cpp
    anbox/container/management_api_stub.cpp
    anbox/container/management_api_skeleton.cpp
    anbox/container/management_api_message_processor.cpp

    anbox/network/credentials.cpp
    anbox/network/message_sender.h
    anbox/network/message_receiver.h
    anbox/network/message_processor.h
    anbox/network/connector.h
    anbox/network/connection_creator.h
    anbox/network/published_socket_connector.cpp
    anbox/network/connections.h
    anbox/network/socket_connection.cpp
    anbox/network/socket_messenger.cpp
    anbox/network/delegate_message_processor.cpp
    anbox/network/delegate_connection_creator.h
    anbox/network/base_socket_messenger.cpp
    anbox/network/local_socket_messenger.cpp
    anbox/network/tcp_socket_messenger.cpp
    anbox/network/socket_helper.cpp
    anbox/network/tcp_socket_connector.cpp

    anbox/rpc/channel.cpp
    anbox/rpc/pending_call_cache.cpp
    anbox/rpc/constants.h
    anbox/rpc/connection_creator.cpp
    anbox/rpc/message_processor.cpp
    anbox/rpc/template_message_processor.h
    anbox/rpc/make_protobuf_object.h

    anbox/graphics/opengles_message_processor.cpp
    anbox/graphics/buffer_queue.cpp
    anbox/graphics/buffered_io_stream.cpp
    anbox/graphics/gl_renderer_server.cpp
    anbox/graphics/density.cpp
    anbox/graphics/density.h
    anbox/graphics/rect.cpp
    anbox/graphics/layer_composer.cpp
    anbox/graphics/multi_window_composer_strategy.cpp
    anbox/graphics/single_window_composer_strategy.cpp
    anbox/graphics/program_family.cpp
    anbox/graphics/primitives.h
    anbox/graphics/renderer.h
    anbox/graphics/gl_extensions.h

    anbox/graphics/emugl/ColorBuffer.cpp
    anbox/graphics/emugl/DisplayManager.cpp
    anbox/graphics/emugl/RendererConfig.cpp
    anbox/graphics/emugl/Renderable.cpp
    anbox/graphics/emugl/Renderer.cpp
    anbox/graphics/emugl/ReadBuffer.cpp
    anbox/graphics/emugl/RenderApi.cpp
    anbox/graphics/emugl/RenderContext.cpp
    anbox/graphics/emugl/RenderControl.cpp
    anbox/graphics/emugl/RenderThread.cpp
    anbox/graphics/emugl/RenderThreadInfo.cpp
    anbox/graphics/emugl/TextureDraw.cpp
    anbox/graphics/emugl/TextureResize.cpp
    anbox/graphics/emugl/TimeUtils.cpp
    anbox/graphics/emugl/WindowSurface.cpp

    anbox/audio/server.cpp
    anbox/audio/client_info.h
    anbox/audio/source.h
    anbox/audio/sink.h

    anbox/wm/display.cpp
    anbox/wm/task.cpp
    anbox/wm/stack.cpp
    anbox/wm/manager.cpp
    anbox/wm/single_window_manager.cpp
    anbox/wm/multi_window_manager.cpp
    anbox/wm/window_state.cpp
    anbox/wm/window.cpp

    anbox/platform/base_platform.cpp

    anbox/platform/null/platform.cpp

    anbox/platform/sdl/sdl_wrapper.h
    anbox/platform/sdl/window.cpp
    anbox/platform/sdl/keycode_converter.cpp
    anbox/platform/sdl/platform.cpp
    anbox/platform/sdl/audio_sink.cpp

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
    anbox/qemu/adb_message_processor.cpp

    anbox/bridge/platform_message_processor.cpp
    anbox/bridge/platform_api_skeleton.cpp
    anbox/bridge/android_api_stub.cpp

    anbox/dbus/bus.cpp
    anbox/dbus/bus.h
    anbox/dbus/sd_bus_helpers.h
    anbox/dbus/interface.h
    anbox/dbus/skeleton/service.cpp
    anbox/dbus/skeleton/service.h
    anbox/dbus/skeleton/application_manager.cpp
    anbox/dbus/skeleton/application_manager.h
    anbox/dbus/stub/application_manager.cpp
    anbox/dbus/stub/application_manager.h

    anbox/application/launcher_storage.cpp
    anbox/application/database.cpp
    anbox/application/manager.h

    anbox/cmds/version.cpp
    anbox/cmds/session_manager.cpp
    anbox/cmds/container_manager.cpp
    anbox/cmds/launch.cpp
    anbox/cmds/system_info.cpp
    anbox/cmds/wait_ready.cpp
    anbox/cmds/wait_ready.h

    anbox/utils/environment_file.cpp

    anbox/ui/splash_screen.cpp

    anbox/do_not_copy_or_move.h
    anbox/optional.h
    anbox/defer_action.h)

add_library(anbox-core STATIC ${SOURCES})
target_link_libraries(anbox-core
  ${Boost_LDFLAGS}
  ${Boost_LIBRARIES}
  ${DBUS_CPP_LDFLAGS}
  ${DBUS_CPP_LIBRARIES}
  ${SDL2_LDFLAGS}
  ${SDL2_LIBRARIES}
  ${SDL2_IMAGE_LDFLAGS}
  ${SDL2_IMAGE_LIBRARIES}
  ${LXC_LDFLAGS}
  ${LXC_LIBRARIES}
  ${MIRCLIENT_LDFLAGS}
  ${MIRCLIENT_LIBRARIES}
  ${LIBSYSTEMD_LDFLAGS}
  ${LIBSYSTEMD_LIBRARIES}
  pthread
  process-cpp
  emugl_common
  GLESv1_dec
  GLESv2_dec
  renderControl_dec
  OpenGLESDispatch
  OpenglCodecCommon
  anbox-protobuf
  xdg)

add_executable(anbox main.cpp ${BACKWARD_ENABLE})
target_link_libraries(anbox
    anbox-core)
add_backward(anbox)

install(
  TARGETS anbox
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)
