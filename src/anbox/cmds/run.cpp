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

#include "anbox/logger.h"
#include "anbox/runtime.h"
#include "anbox/container.h"
#include "anbox/config.h"
#include "anbox/pid_persister.h"
#include "anbox/common/dispatcher.h"
#include "anbox/cmds/run.h"
#include "anbox/network/published_socket_connector.h"
#include "anbox/qemu/pipe_connection_creator.h"
#include "anbox/graphics/gl_renderer_server.h"
#include "anbox/input/manager.h"
#include "anbox/bridge/connection_creator.h"
#include "anbox/bridge/platform_message_processor.h"
#include "anbox/bridge/rpc_channel.h"
#include "anbox/bridge/android_api_stub.h"
#include "anbox/ubuntu/platform_api_skeleton.h"
#include "anbox/ubuntu/window_creator.h"
#include "anbox/dbus/skeleton/service.h"

#include <sys/prctl.h>

#include <core/dbus/bus.h>
#include <core/dbus/asio/executor.h>

namespace fs = boost::filesystem;

namespace {
class NullConnectionCreator : public anbox::network::ConnectionCreator {
public:
    void create_connection_for(
            std::shared_ptr<boost::asio::local::stream_protocol::socket> const& socket) override {
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

anbox::cmds::Run::Run(const BusFactory& bus_factory)
    : CommandWithFlagsAndAction{cli::Name{"run"}, cli::Usage{"run"}, cli::Description{"Run the the anbox system"}},
      bus_factory_(bus_factory)
{
    flag(cli::make_flag(cli::Name{"rootfs"}, cli::Description{"Path to Android rootfs"}, rootfs_));
    // Just for the purpose to allow QtMir (or unity8) to find this on our /proc/*/cmdline
    // for proper confinement etc.
    flag(cli::make_flag(cli::Name{"desktop_file_hint"}, cli::Description{"Desktop file hint for QtMir/Unity8"}, desktop_file_hint_));
    flag(cli::make_flag(cli::Name{"apk"}, cli::Description{"Android application to install on startup"}, apk_));
    flag(cli::make_flag(cli::Name{"package"}, cli::Description{"Specifies the package the activity should be launched from."}, package_));
    flag(cli::make_flag(cli::Name{"activity"}, cli::Description{"Activity to start from specified package"}, activity_));
    flag(cli::make_flag(cli::Name{"icon"}, cli::Description{"Icon of the application to run"}, icon_));

    action([this](const cli::Command::Context &ctx) {
        if (rootfs_.empty() || !fs::is_directory(fs::path(rootfs_))) {
            ctx.cout << "Not valid rootfs path provided" << std::endl;
            return EXIT_FAILURE;
        }

        auto trap = core::posix::trap_signals_for_process({core::posix::Signal::sig_term,
                                                           core::posix::Signal::sig_int});
        trap->signal_raised().connect([trap](const core::posix::Signal &signal) {
            INFO("Signal %i received. Good night.", static_cast<int>(signal));
            trap->stop();
        });

        auto rt = Runtime::create();
        auto dispatcher = anbox::common::create_dispatcher_for_runtime(rt);

        auto input_manager = std::make_shared<input::Manager>(rt);

        auto window_creator = std::make_shared<ubuntu::WindowCreator>(input_manager);
        auto renderer = std::make_shared<graphics::GLRendererServer>(window_creator);
        renderer->start();

        // Socket which will be used by the qemud service inside the Android
        // container for things like sensors, vibrtator etc.
        auto qemud_connector = std::make_shared<network::PublishedSocketConnector>(
            utils::string_format("%s/qemud", config::data_path()),
            rt,
            std::make_shared<NullConnectionCreator>());

        // The qemu pipe is used as a very fast communication channel between guest
        // and host for things like the GLES emulation/translation, the RIL or ADB.
        auto qemu_pipe_connector = std::make_shared<network::PublishedSocketConnector>(
            utils::string_format("%s/qemu_pipe", config::data_path()),
            rt,
            std::make_shared<qemu::PipeConnectionCreator>(rt,
                                                          renderer->socket_path(),
                                                          icon_));

        auto android_api_stub = std::make_shared<bridge::AndroidApiStub>();

        auto bridge_connector = std::make_shared<network::PublishedSocketConnector>(
            utils::string_format("%s/anbox_bridge", config::data_path()),
            rt,
            std::make_shared<bridge::ConnectionCreator>(rt,
                [&](const std::shared_ptr<network::MessageSender> &sender) {
                    auto pending_calls = std::make_shared<bridge::PendingCallCache>();
                    auto rpc_channel = std::make_shared<bridge::RpcChannel>(pending_calls, sender);
                    // This is safe as long as we only support a single client. If we support
                    // more than one one day we need proper dispatching to the right one.
                    android_api_stub->set_rpc_channel(rpc_channel);

                    dispatcher->dispatch([&]() {
                        // FIXME make this configurable or once we have a bridge let the host
                        // act as a DNS proxy.
                        android_api_stub->set_dns_servers("anbox", std::vector<std::string>{ "8.8.8.8" });
                    });

                    auto server = std::make_shared<ubuntu::PlatformApiSekeleton>(pending_calls);
                    // FIXME Implement a delegate or use signals from properties-cpp
                    server->on_boot_finished([&](){
                        DEBUG("Boot finished.");
                        dispatcher->dispatch([&]() {
                            if (!apk_.empty()) {
                                DEBUG("Installing %s ..", apk_);
                                android_api_stub->install(apk_);
                            }
                            if (!package_.empty() || !activity_.empty())
                                android_api_stub->launch(package_, activity_);
                        });
                    });

                    return std::make_shared<bridge::PlatformMessageProcessor>(sender, server, pending_calls);
                }));

        auto spec = Container::Spec::Default();
        spec.rootfs_path = rootfs_;
        spec.bind_paths.insert({qemud_connector->socket_file(), "/dev/qemud"});
        spec.bind_paths.insert({qemu_pipe_connector->socket_file(), "/dev/qemu_pipe"});
        spec.bind_paths.insert({bridge_connector->socket_file(), "/dev/anbox_bridge"});

        input_manager->generate_mappings(spec.bind_paths);

        // A place where we can exchange files with the container
        spec.bind_paths.insert({config::host_share_path(), config::container_share_path()});

        // FIXME(morphis): those directories should be really somewhere on our
        // persistent data directory so we keep any runtime data accross restarts.
        spec.temporary_dirs.push_back("/data");
        spec.temporary_dirs.push_back("/cache");
        spec.temporary_dirs.push_back("/storage");

        spec.temporary_dirs.push_back("/dev/input");

        // NOTE: We're not mapping /dev/alarm here as if its not available
        // Android will automatically use its timerfd based fallback
        // implementation instead.

        // We isolate the container from accessing binder nodes of the host
        // through the IPC namespace which gets support for binder with extra
        // patches we require.
        spec.dev_bind_paths.push_back("/dev/binder");
        // Required for shared memory allocations. TODO(morphis): Letting the guest
        // access should be ok but needs more investigation.
        spec.dev_bind_paths.push_back("/dev/ashmem");

        auto container = Container::create(spec);

        auto bus = bus_factory_();
        bus->install_executor(core::dbus::asio::make_executor(bus, rt->service()));

        auto skeleton = anbox::dbus::skeleton::Service::create_for_bus(bus, android_api_stub);

        rt->start();
        container->start();

        trap->run();

        container->stop();
        rt->stop();

        return EXIT_SUCCESS;
    });
}
