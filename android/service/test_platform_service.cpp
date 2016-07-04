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

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include "android/service/platform_service_interface.h"

int main(int, char**) {
    auto binder = android::defaultServiceManager()->getService(android::String16("anbox.PlatformService"));
    if (not binder.get())
        return 0;

    android::ProcessState::self()->startThreadPool();

    android::sp<android::BpPlatformService> platform_service = new android::BpPlatformService(binder);

    return platform_service->boot_finished();
}
