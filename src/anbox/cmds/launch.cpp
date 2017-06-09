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

#include "anbox/cmds/launch.h"
#include "anbox/common/wait_handle.h"
#include "anbox/dbus/stub/application_manager.h"
#include "anbox/common/dispatcher.h"
#include "anbox/ui/splash_screen.h"
#include "anbox/config.h"
#include "anbox/runtime.h"
#include "anbox/logger.h"

#include <core/dbus/asio/executor.h>

#include <boost/filesystem.hpp>

#include "core/posix/exec.h"
#include "core/posix/fork.h"
#include "core/posix/signal.h"

namespace fs = boost::filesystem;

namespace {
const boost::posix_time::seconds max_wait_timeout{240};
const int max_restart_attempts{3};
const std::chrono::seconds restart_interval{5};
}

bool anbox::cmds::Launch::try_launch_activity(const std::shared_ptr<dbus::stub::ApplicationManager> &stub) {
  try {
    DEBUG("Sending launch intent %s to Android ..", intent_);
    stub->launch(intent_, graphics::Rect::Invalid, stack_);
  } catch (const std::exception &err) {
    ERROR("Failed to launch activity: %s", err.what());
    return false;
  } catch (...) {
    ERROR("Failed to launch activity");
    return false;
  }

  return true;
}

anbox::cmds::Launch::Launch()
    : CommandWithFlagsAndAction{
          cli::Name{"launch"}, cli::Usage{"launch"},
          cli::Description{"Launch an Activity by sending an intent"}} {
  flag(cli::make_flag(cli::Name{"action"},
                      cli::Description{"Action of the intent"},
                      intent_.action));
  flag(cli::make_flag(cli::Name{"type"},
                      cli::Description{"MIME type for the intent"},
                      intent_.type));
  flag(cli::make_flag(cli::Name{"uri"},
                      cli::Description{"URI used as data within the intent"},
                      intent_.uri));
  flag(cli::make_flag(cli::Name{"package"},
                      cli::Description{"Package the intent should go to"},
                      intent_.package));
  flag(cli::make_flag(cli::Name{"component"},
                      cli::Description{"Component of a package the intent should go"},
                      intent_.component));
  flag(cli::make_flag(cli::Name{"stack"},
                      cli::Description{"Which window stack the activity should be started on. Possible: default, fullscreen, freeform"},
                      stack_));

  action([this](const cli::Command::Context&) {
    if (!intent_.valid()) {
      ERROR("The intent you provided is invalid. Please provide a correct launch intent.");
      return EXIT_FAILURE;
    }

    auto trap = core::posix::trap_signals_for_process({core::posix::Signal::sig_term, core::posix::Signal::sig_int});
    trap->signal_raised().connect([trap](const core::posix::Signal& signal) {
      INFO("Signal %i received. Good night.", static_cast<int>(signal));
      trap->stop();
    });

    auto rt = Runtime::create();

    auto bus = std::make_shared<core::dbus::Bus>(core::dbus::WellKnownBus::session);
    bus->install_executor(core::dbus::asio::make_executor(bus, rt->service()));

    const auto snap_path = utils::get_env_value("SNAP");
    if (!snap_path.empty()) {
      const auto resource_path = fs::path(snap_path) / "usr" / "share" / "anbox";
      SystemConfiguration::instance().set_resource_path(resource_path);
    }

    std::shared_ptr<ui::SplashScreen> ss;

    // Instead of relying on the user session init system to start our
    // session manager process we also attempt to start it on our own
    // if not already running. This will help to mitigate problems with
    // a crashing or a not yet started session manager instance.
    std::shared_ptr<dbus::stub::ApplicationManager> stub;
    for (auto n = 0; n < max_restart_attempts; n++) {
      try {
        stub = dbus::stub::ApplicationManager::create_for_bus(bus);
        break;
      } catch (std::exception &err) {
        WARNING("Anbox session manager service isn't running, trying to start it.");

        // Give us a splash screen as long as we're trying to connect
        // with the session manager so the user knows something is
        // happening after he started Anbox.
        if (!ss)
          ss = std::make_shared<ui::SplashScreen>();

        std::vector<std::string> args = {"session-manager"};

        std::map<std::string,std::string> env;
        core::posix::this_process::env::for_each([&](const std::string &name, const std::string &value) {
          env.insert({name, value});
        });

        const auto exe_path = utils::process_get_exe_path(::getpid());
        if (!fs::exists(exe_path)) {
          ERROR("Can't find correct anbox executable to run. Found %s but does not exist", exe_path);
          return EXIT_FAILURE;
        }

        try {
          auto flags = core::posix::StandardStream::stdout | core::posix::StandardStream::stderr;
          // If we have logging enable in debug mode then we allow the child process
          // to print to stdout/stderr too.
          if (Log().GetSeverity() == Logger::Severity::kDebug)
            flags = core::posix::StandardStream::empty;
          auto child = core::posix::fork([&]() {
            auto grandchild = core::posix::exec(exe_path, args, env, flags);
            grandchild.dont_kill_on_cleanup();
            return core::posix::exit::Status::success;
          }, flags);

          // We don't wait for the grandchild but the child as we use double forking
          // here to break through the process hierarchy and make the grandchild a
          // direct child of the init process so it keeps running on its own and
          // indepent of our short living process here.
          child.wait_for(core::posix::wait::Flags::untraced);

          DEBUG("Started session manager, will now try to connect ..");
        }
        catch (...) {
          ERROR("Failed to start session manager instance");
        }

        std::this_thread::sleep_for(restart_interval);
      }
    }

    if (!stub) {
      ERROR("Couldn't get a connection with the session manager");
      return EXIT_FAILURE;
    }

    bool success = false;
    auto dispatcher = anbox::common::create_dispatcher_for_runtime(rt);
    dispatcher->dispatch([&]() {
      if (stub->ready()) {
        ss.reset();
        success = try_launch_activity(stub);
        trap->stop();
        return;
      }

      DEBUG("Android hasn't fully booted yet. Waiting a bit ..");

      stub->ready().changed().connect([&](bool ready) {
        if (!ready)
          return;
        ss.reset();
        success = try_launch_activity(stub);
        trap->stop();
      });
    });

    boost::asio::deadline_timer timer(rt->service());
    timer.expires_from_now(max_wait_timeout);
    timer.async_wait([&](const boost::system::error_code&) {
      WARNING("Stopped waiting as we've already waited for too long. Something");
      WARNING("is wrong with your setup or the container has failed to boot.");
      WARNING("If you think you found a bug please don't hesitate to file on");
      WARNING("at https://github.com/anbox/anbox/issues/new");
      trap->stop();
    });

    rt->start();
    trap->run();
    rt->stop();

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
  });
}
