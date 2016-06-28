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

#include <sys/resource.h>
#include <cutils/sched_policy.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>

#include <binder/IServiceManager.h>
#include <binder/BinderService.h>
#include <gui/ISurfaceComposer.h>
#include <gui/IDisplayEventConnection.h>
#include <binder/IPermissionController.h>
#include <binder/MemoryHeapBase.h>

#include "surface_composer.h"

using namespace android;

int main(int, char**) {
    ProcessState::self()->setThreadPoolMaxThreadCount(4);

    sp<ProcessState> ps(ProcessState::self());
    ps->startThreadPool();

    setpriority(PRIO_PROCESS, 0, PRIORITY_URGENT_DISPLAY);
    set_sched_policy(0, SP_FOREGROUND);

    anbox::android::SurfaceComposer::instantiate();

    IPCThreadState::self()->joinThreadPool();

    return EXIT_SUCCESS;
}
