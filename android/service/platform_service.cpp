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

#include "android/service/platform_service.h"
#include "android/service/platform_api_stub.h"
#include "anbox/bridge/rpc_channel.h"

#define LOG_TAG "Anboxd"
#include <cutils/log.h>

using namespace android;

namespace android {
PlatformService::PlatformService(const std::shared_ptr<anbox::PlatformApiStub> &platform_api_stub) :
    platform_api_stub_(platform_api_stub) {
}

status_t PlatformService::boot_finished() {
    ALOGI("!!!!!!!!!!!!!!!!!!!!!!! BOOOT FINISHED --- ANBOXD");
    platform_api_stub_->boot_finished();
    return OK;
}
} // namespace android
