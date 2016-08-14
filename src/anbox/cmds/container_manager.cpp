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

#include "anbox/cmds/container_manager.h"
#include "anbox/container/service.h"
#include "anbox/runtime.h"
#include "anbox/logger.h"

#include "core/posix/signal.h"

anbox::cmds::ContainerManager::ContainerManager()
    : CommandWithFlagsAndAction{cli::Name{"container-manager"}, cli::Usage{"container-manager"}, cli::Description{"Start the container manager service"}}
{
    action([](const cli::Command::Context& ctxt) {
        auto trap = core::posix::trap_signals_for_process({core::posix::Signal::sig_term,
                                                           core::posix::Signal::sig_int});
        trap->signal_raised().connect([trap](const core::posix::Signal &signal) {
            INFO("Signal %i received. Good night.", static_cast<int>(signal));
            trap->stop();
        });

        auto rt = Runtime::create();
        auto service = container::Service::create(rt);

        rt->start();
        trap->run();
        rt->stop();

        return 0;
    });
}
