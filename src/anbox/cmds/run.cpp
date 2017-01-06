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

#include <boost/filesystem.hpp>

#include "core/posix/signal.h"

#include "anbox/application/launcher_storage.h"
#include "anbox/bridge/android_api_stub.h"
#include "anbox/bridge/platform_api_skeleton.h"
#include "anbox/bridge/platform_message_processor.h"
#include "anbox/cmds/run.h"
#include "anbox/common/dispatcher.h"
#include "anbox/config.h"
#include "anbox/container/client.h"
#include "anbox/dbus/skeleton/service.h"
#include "anbox/graphics/gl_renderer_server.h"
#include "anbox/input/manager.h"
#include "anbox/logger.h"
#include "anbox/network/published_socket_connector.h"
#include "anbox/qemu/pipe_connection_creator.h"
#include "anbox/rpc/channel.h"
#include "anbox/rpc/connection_creator.h"
#include "anbox/runtime.h"
#include "anbox/ubuntu/platform_policy.h"
#include "anbox/wm/manager.h"

#include "external/xdg/xdg.h"

#include <sys/prctl.h>

#include <core/dbus/asio/executor.h>
#include <core/dbus/bus.h>

namespace fs = boost::filesystem;

namespace {
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

anbox::cmds::Run::BusFactory anbox::cmds::Run::session_bus_factory() {
  return []() {
    return std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);
  };
}

anbox::cmds::Run::Run(const BusFactory &bus_factory)
    : CommandWithFlagsAndAction{cli::Name{"run"}, cli::Usage{"run"},
                                cli::Description{"Run the the anbox system"}},
      bus_factory_(bus_factory) {
  // Just for the purpose to allow QtMir (or unity8) to find this on our
  // /proc/*/cmdline
  // for proper confinement etc.
  flag(cli::make_flag(cli::Name{"desktop_file_hint"},
                      cli::Description{"Desktop file hint for QtMir/Unity8"},
                      desktop_file_hint_));
  flag(cli::make_flag(cli::Name{"icon"},
                      cli::Description{"Icon of the application to run"},
                      icon_));

  action([this](const cli::Command::Context &ctx) {
    auto trap = core::posix::trap_signals_for_process(
        {core::posix::Signal::sig_term, core::posix::Signal::sig_int});
    trap->signal_raised().connect([trap](const core::posix::Signal &signal) {
      INFO("Signal %i received. Good night.", static_cast<int>(signal));
      trap->stop();
    });

    utils::ensure_paths({
        config::socket_path(), config::host_input_device_path(),
    });

    auto rt = Runtime::create();
    auto dispatcher = anbox::common::create_dispatcher_for_runtime(rt);

    container::Client container(rt);
    container.register_terminate_handler([&]() {
      WARNING("Lost connection to container manager, terminating.");
      trap->stop();
    });

    auto input_manager = std::make_shared<input::Manager>(rt);

    auto android_api_stub = std::make_shared<bridge::AndroidApiStub>();

    auto policy = std::make_shared<ubuntu::PlatformPolicy>(input_manager,
                                                           android_api_stub);
    // FIXME this needs to be removed and solved differently behind the scenes
    registerDisplayManager(policy);

    auto window_manager = std::make_shared<wm::Manager>(policy);

    auto launcher_storage = std::make_shared<application::LauncherStorage>(
        xdg::data().home() / "applications" / "anbox",
        xdg::data().home() / "anbox" / "icons");

    auto gl_server =
        std::make_shared<graphics::GLRendererServer>(window_manager);

    policy->set_renderer(gl_server->renderer());

    // The qemu pipe is used as a very fast communication channel between guest
    // and host for things like the GLES emulation/translation, the RIL or ADB.
    auto qemu_pipe_connector =
        std::make_shared<network::PublishedSocketConnector>(
            utils::string_format("%s/qemu_pipe", config::socket_path()), rt,
            std::make_shared<qemu::PipeConnectionCreator>(gl_server->renderer(), rt));

    auto bridge_connector = std::make_shared<network::PublishedSocketConnector>(
        utils::string_format("%s/anbox_bridge", config::socket_path()), rt,
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
                  pending_calls, window_manager, launcher_storage);
              server->register_boot_finished_handler(
                  [&]() { DEBUG("Android successfully booted"); });
              return std::make_shared<bridge::PlatformMessageProcessor>(
                  sender, server, pending_calls);
            }));

    container::Configuration container_configuration;
    container_configuration.bind_mounts = {
        {qemu_pipe_connector->socket_file(), "/dev/qemu_pipe"},
        {bridge_connector->socket_file(), "/dev/anbox_bridge"},
        {config::host_input_device_path(), "/dev/input"},
        {"/dev/binder", "/dev/binder"},
        {"/dev/ashmem", "/dev/ashmem"},
        {"/dev/fuse", "/dev/fuse"},
    };

    dispatcher->dispatch(
        [&]() { container.start_container(container_configuration); });

    auto bus = bus_factory_();
    bus->install_executor(core::dbus::asio::make_executor(bus, rt->service()));

    auto skeleton =
        anbox::dbus::skeleton::Service::create_for_bus(bus, android_api_stub);

    rt->start();
    trap->run();
    rt->stop();

    return EXIT_SUCCESS;
  });
}
