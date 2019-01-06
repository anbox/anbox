/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <boost/filesystem.hpp>

#include "core/posix/signal.h"

#include "anbox/application/launcher_storage.h"
#include "anbox/application/database.h"
#include "anbox/audio/server.h"
#include "anbox/bridge/android_api_stub.h"
#include "anbox/bridge/platform_api_skeleton.h"
#include "anbox/bridge/platform_message_processor.h"
#include "anbox/graphics/gl_renderer_server.h"

#include "anbox/cmds/session_manager.h"
#include "anbox/common/dispatcher.h"
#include "anbox/system_configuration.h"
#include "anbox/container/client.h"
#include "anbox/dbus/bus.h"
#include "anbox/dbus/skeleton/service.h"
#include "anbox/input/manager.h"
#include "anbox/logger.h"
#include "anbox/network/published_socket_connector.h"
#include "anbox/qemu/pipe_connection_creator.h"
#include "anbox/rpc/channel.h"
#include "anbox/rpc/connection_creator.h"
#include "anbox/runtime.h"
#include "anbox/platform/base_platform.h"
#include "anbox/wm/multi_window_manager.h"
#include "anbox/wm/single_window_manager.h"

#include "external/xdg/xdg.h"

#include <sys/prctl.h>

#pragma GCC diagnostic pop

namespace fs = boost::filesystem;

namespace {
constexpr const char *default_appmgr_package{"org.anbox.appmgr"};
constexpr const char *default_appmgr_component{"org.anbox.appmgr.AppViewActivity"};
const boost::posix_time::milliseconds default_appmgr_startup_delay{50};
const anbox::graphics::Rect default_single_window_size{0, 0, 1024, 768};

class NullConnectionCreator : public anbox::network::ConnectionCreator<
                                  boost::asio::local::stream_protocol> {
 public:
  void create_connection_for(
      std::shared_ptr<boost::asio::local::stream_protocol::socket> const
          &socket) override {
    WARNING("Not implemented");
    socket->close();
  }
};
}

void anbox::cmds::SessionManager::launch_appmgr_if_needed(const std::shared_ptr<bridge::AndroidApiStub> &android_api_stub) {
  if (!single_window_)
    return;

  android::Intent launch_intent;
  launch_intent.package = default_appmgr_package;
  launch_intent.component = default_appmgr_component;
  // As this will only be executed in single window mode we don't have
  // to specify and launch bounds.
  android_api_stub->launch(launch_intent, graphics::Rect::Invalid, wm::Stack::Id::Default);
}

anbox::cmds::SessionManager::SessionManager()
    : CommandWithFlagsAndAction{cli::Name{"session-manager"}, cli::Usage{"session-manager"},
                                cli::Description{"Run the the anbox session manager"}},
      window_size_(default_single_window_size) {
  // Just for the purpose to allow QtMir (or unity8) to find this on our
  // /proc/*/cmdline
  // for proper confinement etc.
  flag(cli::make_flag(cli::Name{"desktop_file_hint"},
                      cli::Description{"Desktop file hint for QtMir/Unity8"},
                      desktop_file_hint_));
  flag(cli::make_flag(cli::Name{"single-window"},
                      cli::Description{"Start in single window mode."},
                      single_window_));
  flag(cli::make_flag(cli::Name{"window-size"},
                      cli::Description{"Size of the window in single window mode, e.g. --window-size=1024,768"},
                      window_size_));
  flag(cli::make_flag(cli::Name{"standalone"},
                      cli::Description{"Prevents the Container Manager from starting the default container (Experimental)"},
                      standalone_));
  flag(cli::make_flag(cli::Name{"experimental"},
                      cli::Description{"Allows users to use experimental features"},
                      experimental_));
  flag(cli::make_flag(cli::Name{"use-system-dbus"},
                      cli::Description{"Use system instead of session DBus"},
                      use_system_dbus_));
  flag(cli::make_flag(cli::Name{"software-rendering"},
                      cli::Description{"Use software rendering instead of hardware accelerated GL rendering"},
                      use_software_rendering_));

  action([this](const cli::Command::Context &) {
    auto trap = core::posix::trap_signals_for_process(
        {core::posix::Signal::sig_term, core::posix::Signal::sig_int});
    trap->signal_raised().connect([trap](const core::posix::Signal &signal) {
      INFO("Signal %i received. Good night.", static_cast<int>(signal));
      trap->stop();
    });

    if (standalone_ && !experimental_) {
      ERROR("Experimental features selected, but --experimental flag not set");
      return EXIT_FAILURE;
    }

    if (!fs::exists("/dev/binder") || !fs::exists("/dev/ashmem")) {
      ERROR("Failed to start as either binder or ashmem kernel drivers are not loaded");
      return EXIT_FAILURE;
    }

    utils::ensure_paths({
        SystemConfiguration::instance().socket_dir(),
        SystemConfiguration::instance().input_device_dir(),
    });

    auto rt = Runtime::create();
    auto dispatcher = anbox::common::create_dispatcher_for_runtime(rt);

    if (!standalone_) {
      container_ = std::make_shared<container::Client>(rt);
      container_->register_terminate_handler([&]() {
        WARNING("Lost connection to container manager, terminating.");
        trap->stop();
      });
    }

    auto input_manager = std::make_shared<input::Manager>(rt);
    auto android_api_stub = std::make_shared<bridge::AndroidApiStub>();

    auto display_frame = graphics::Rect::Invalid;
    if (single_window_)
      display_frame = window_size_;

    auto platform = platform::create(utils::get_env_value("ANBOX_PLATFORM", "sdl"),
                                     input_manager,
                                     display_frame,
                                     single_window_);
    if (!platform) 
      return EXIT_FAILURE;

    auto app_db = std::make_shared<application::Database>();

    std::shared_ptr<wm::Manager> window_manager;
    bool using_single_window = false;
    if (platform->supports_multi_window() && !single_window_)
      window_manager = std::make_shared<wm::MultiWindowManager>(platform, android_api_stub, app_db);
    else {
      window_manager = std::make_shared<wm::SingleWindowManager>(platform, display_frame, app_db);
      using_single_window = true;
    }

    const auto should_force_software_rendering = utils::get_env_value("ANBOX_FORCE_SOFTWARE_RENDERING", "false");
    auto gl_driver = graphics::GLRendererServer::Config::Driver::Host;
    if (should_force_software_rendering == "true" || use_software_rendering_)
     gl_driver = graphics::GLRendererServer::Config::Driver::Software;

    graphics::GLRendererServer::Config renderer_config {
      gl_driver,
      single_window_
    };
    auto gl_server = std::make_shared<graphics::GLRendererServer>(renderer_config, window_manager);

    platform->set_window_manager(window_manager);
    platform->set_renderer(gl_server->renderer());
    window_manager->setup();

    auto app_manager = std::static_pointer_cast<application::Manager>(android_api_stub);
    if (!using_single_window) {
      // When we're not running single window mode we need to restrict ourself to
      // only launch applications in freeform mode as otherwise the window tracking
      // doesn't work.
      app_manager = std::make_shared<application::RestrictedManager>(
            android_api_stub, wm::Stack::Id::Freeform);
    }

    auto audio_server = std::make_shared<audio::Server>(rt, platform);

    const auto socket_path = SystemConfiguration::instance().socket_dir();

    // The qemu pipe is used as a very fast communication channel between guest
    // and host for things like the GLES emulation/translation, the RIL or ADB.
    auto qemu_pipe_connector =
        std::make_shared<network::PublishedSocketConnector>(
            utils::string_format("%s/qemu_pipe", socket_path), rt,
            std::make_shared<qemu::PipeConnectionCreator>(gl_server->renderer(), rt));

    boost::asio::deadline_timer appmgr_start_timer(rt->service());

    auto bridge_connector = std::make_shared<network::PublishedSocketConnector>(
        utils::string_format("%s/anbox_bridge", socket_path), rt,
        std::make_shared<rpc::ConnectionCreator>(
            rt, [&](const std::shared_ptr<network::MessageSender> &sender) {
              auto pending_calls = std::make_shared<rpc::PendingCallCache>();
              auto rpc_channel =
                  std::make_shared<rpc::Channel>(pending_calls, sender);
              // This is safe as long as we only support a single client. If we
              // support
              // more than one one day we need proper dispatching to the right
              // one.
              android_api_stub->set_rpc_channel(rpc_channel);

              auto server = std::make_shared<bridge::PlatformApiSkeleton>(
                  pending_calls, platform, window_manager, app_db);
              server->register_boot_finished_handler([&]() {
                DEBUG("Android successfully booted");
                android_api_stub->ready().set(true);
                appmgr_start_timer.expires_from_now(default_appmgr_startup_delay);
                appmgr_start_timer.async_wait([&](const boost::system::error_code &err) {
                  if (err != boost::system::errc::success)
                    return;
                  launch_appmgr_if_needed(android_api_stub);
                });
              });
              return std::make_shared<bridge::PlatformMessageProcessor>(
                  sender, server, pending_calls);
            }));

    container::Configuration container_configuration;

    // Instruct healthd to fake battery level as it may take it from other connected
    // devices like mouse or keyboard and will incorrectly show a system popup to
    // shutdown the Android system because of low battery. This prevents any kind of
    // input as focus is bound to the system popup exclusively.
    //
    // See https://github.com/anbox/anbox/issues/780 for further details.
    container_configuration.extra_properties.push_back("ro.boot.fake_battery=1");

    if (!standalone_) {
      container_configuration.bind_mounts = {
        {qemu_pipe_connector->socket_file(), "/dev/qemu_pipe"},
        {bridge_connector->socket_file(), "/dev/anbox_bridge"},
        {audio_server->socket_file(), "/dev/anbox_audio"},
        {SystemConfiguration::instance().input_device_dir(), "/dev/input"},

      };

      container_configuration.devices = {
        {"/dev/binder", {0666}},
        {"/dev/ashmem", {0666}},
        {"/dev/fuse", {0666}},
      };

      dispatcher->dispatch([&]() {
        container_->start(container_configuration);
      });
    }

    auto bus_type = anbox::dbus::Bus::Type::Session;
    if (use_system_dbus_)
      bus_type = anbox::dbus::Bus::Type::System;
    auto bus = std::make_shared<anbox::dbus::Bus>(bus_type);

    auto skeleton = anbox::dbus::skeleton::Service::create_for_bus(bus, app_manager);

    bus->run_async();

    rt->start();
    trap->run();

    if (!standalone_) {
      // Stop the container which should close all open connections we have on
      // our side and should terminate all services.
      container_->stop();
    }

    rt->stop();

    return EXIT_SUCCESS;
  });
}
