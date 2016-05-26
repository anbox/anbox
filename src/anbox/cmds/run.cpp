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
#include "anbox/cmds/run.h"
#include "anbox/network/published_socket_connector.h"
#include "anbox/network/qemu_pipe_connection_creator.h"
#include "anbox/graphics/gl_renderer_server.h"

#include <sys/prctl.h>

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

anbox::cmds::Run::Run()
    : CommandWithFlagsAndAction{cli::Name{"run"}, cli::Usage{"run"}, cli::Description{"Run the the anbox system"}}
{
    flag(cli::make_flag(cli::Name{"rootfs"}, cli::Description{"Path to Android rootfs"}, rootfs_));
    // Just for the purpose to allow QtMir (or unity8) to find this on our /proc/*/cmdline
    // for proper confinement etc.
    flag(cli::make_flag(cli::Name{"desktop_file_hint"}, cli::Description{"Desktop file hint for QtMir/Unity8"}, desktop_file_hint_));
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

        auto renderer = std::make_shared<graphics::GLRendererServer>();
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
            std::make_shared<network::QemuPipeConnectionCreator>(rt, renderer->socket_path()));

        auto spec = Container::Spec::Default();
        spec.rootfs_path = rootfs_;
        spec.bind_paths.insert({qemud_connector->socket_file(), "/dev/qemud"});
        spec.bind_paths.insert({qemu_pipe_connector->socket_file(), "/dev/qemu_pipe"});
        spec.temporary_dirs.push_back("/data");
        // We isolate the container from accessing binder nodes of the host
        // through the IPC namespace which gets support for binder with extra
        // patches we require.
        spec.dev_bind_paths.push_back("/dev/binder");
        // Required for shared memory allocations. TODO(morphis): Letting the guest
        // access should be ok but needs more investigation.
        spec.dev_bind_paths.push_back("/dev/ashmem");

        auto container = Container::create(spec);

        rt->start();
        container->start();

        trap->run();

        container->stop();
        rt->stop();

        return EXIT_SUCCESS;
    });
}
