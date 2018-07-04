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
#include "anbox/dbus/stub/application_manager.h"
#include "anbox/dbus/interface.h"
#include "anbox/ui/splash_screen.h"
#include "anbox/system_configuration.h"
#include "anbox/logger.h"

#include "core/posix/exec.h"
#include "core/posix/fork.h"
#include "core/posix/signal.h"

#include <boost/filesystem.hpp>

#include <fcntl.h>
#include <sys/stat.h>

namespace fs = boost::filesystem;

namespace {
constexpr unsigned int max_session_mgr_wait_attempts{10};
const std::chrono::seconds session_mgr_wait_interval{5};
constexpr unsigned int max_dbus_service_wait_attempts{10};
const std::chrono::seconds dbus_service_wait_interval{5};

static int redirect_to_null(int flags, int fd) {
  int fd2;
  if ((fd2 = open("/dev/null", flags)) < 0)
    return -1;

  if (fd2 == fd)
    return fd;

  if (dup2(fd2, fd) < 0)
    return -1;

  close(fd2);
  return fd;
}
} // namespace

bool anbox::cmds::Launch::launch_session_manager() {
  std::vector<std::string> args = {"session-manager"};
  const auto should_force_software_rendering = utils::get_env_value("ANBOX_FORCE_SOFTWARE_RENDERING", "false");
  if (should_force_software_rendering == "true")
    args.push_back("--software-rendering");

  std::map<std::string,std::string> env;
  core::posix::this_process::env::for_each([&](const std::string &name, const std::string &value) {
    env.insert({name, value});
  });

  const auto exe_path = utils::process_get_exe_path(::getpid());
  if (!fs::exists(exe_path)) {
    ERROR("Can't find correct anbox executable to run. Found %s but does not exist", exe_path);
    return false;
  }

  try {
    auto flags = core::posix::StandardStream::empty;
    auto child = core::posix::fork([&]() {

      // We redirect all in/out/err to /dev/null as they can't be seen
      // anywhere. All logging output will directly go to syslog as we
      // will become a session leader below which will get us rid of a
      // controlling terminal.
      if (redirect_to_null(O_RDONLY, 0) < 0 ||
          redirect_to_null(O_WRONLY, 1) < 0 ||
          redirect_to_null(O_WRONLY, 2) < 0) {
        ERROR("Failed to redirect stdout/stderr/stdin: %s", strerror(errno));
        return core::posix::exit::Status::failure;
      }

      // As we forked one time already we're sure that our process is
      // not the session leader anymore so we can safely become the
      // new one and lead the process group.
      if (setsid() < 0) {
        ERROR("Failed to become new session leader: %s", strerror(errno));
        return core::posix::exit::Status::failure;
      }

      umask(0077);

      if (chdir("/") < 0) {
        ERROR("Failed to change current directory: %s", strerror(errno));
        return core::posix::exit::Status::failure;
      }

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

  return true;
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
  flag(cli::make_flag(cli::Name{"use-system-dbus"},
                      cli::Description{"Use system instead of session DBus"},
                      use_system_dbus_));


  action([this](const cli::Command::Context&) {
    if (!intent_.valid()) {
      ERROR("The intent you provided is invalid. Please provide a correct launch intent.");
      ERROR("For example to launch the application manager, run:");
      ERROR("$ anbox launch --package=org.anbox.appmgr --component=org.anbox.appmgr.AppViewActivity");
      return EXIT_FAILURE;
    }

    auto bus_type = anbox::dbus::Bus::Type::Session;
    if (use_system_dbus_)
      bus_type = anbox::dbus::Bus::Type::System;
    auto bus = std::make_shared<anbox::dbus::Bus>(bus_type);

    std::shared_ptr<ui::SplashScreen> ss;
    if (!bus->has_service_with_name(dbus::interface::Service::name())) {
      DEBUG("Session manager is not yet running, trying to start it");

      if (!launch_session_manager())
        return EXIT_FAILURE;

      // Give us a splash screen as long as we're trying to connect
      // with the session manager so the user knows something is
      // happening after he started Anbox.
      ss = std::make_shared<ui::SplashScreen>();
    }

    unsigned int n = 0;
    while (n < max_dbus_service_wait_attempts) {
      if (bus->has_service_with_name(dbus::interface::Service::name()))
        break;

      std::this_thread::sleep_for(dbus_service_wait_interval);
      n++;
    }

    auto app_mgr = dbus::stub::ApplicationManager::create_for_bus(bus);
    n = 0;
    while (n < max_session_mgr_wait_attempts) {
      app_mgr->update_properties();
      if (app_mgr->ready().get())
        break;

      std::this_thread::sleep_for(session_mgr_wait_interval);
      n++;
    }

    if (!app_mgr->ready()) {
      ERROR("Session manager failed to become ready");
      return EXIT_FAILURE;
    }

    // If we have a splash screen now is the time to drop it as we're
    // going to launch the real application now.
    ss.reset();

    const auto success = try_launch_activity(app_mgr);
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
  });
}
