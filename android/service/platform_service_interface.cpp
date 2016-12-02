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

#include "android/service/platform_service_interface.h"

namespace android {
BpPlatformService::BpPlatformService(const sp<IBinder> &binder) :
    BpInterface<IPlatformService>(binder) {
}

status_t BpPlatformService::boot_finished() {
    Parcel data, reply;
    data.writeInterfaceToken(IPlatformService::getInterfaceDescriptor());
    return remote()->transact(IPlatformService::BOOT_FINISHED, data, &reply);
}

status_t BpPlatformService::update_window_state(const Parcel&) {
    Parcel data, reply;
    data.writeInterfaceToken(IPlatformService::getInterfaceDescriptor());
    return remote()->transact(IPlatformService::UPDATE_WINDOW_STATE, data, &reply);
}

status_t BpPlatformService::update_application_list(const Parcel&) {
    Parcel data, reply;
    data.writeInterfaceToken(IPlatformService::getInterfaceDescriptor());
    return remote()->transact(IPlatformService::UPDATE_APPLICATION_LIST, data, &reply);
}

IMPLEMENT_META_INTERFACE(PlatformService, "org.anbox.IPlatformService");

status_t BnPlatformService::onTransact(uint32_t code, const Parcel &data,
                                       Parcel *reply, uint32_t flags) {
    switch (code) {
    case BOOT_FINISHED:
        CHECK_INTERFACE(IPlatformService, data, reply);
        return boot_finished();
    case UPDATE_WINDOW_STATE:
        CHECK_INTERFACE(IPlatformService, data, reply);
        return update_window_state(data);
    case UPDATE_APPLICATION_LIST:
        CHECK_INTERFACE(IPlatformService, data, reply);
        return update_application_list(data);
    default:
        break;
    }

    return BBinder::onTransact(code, data, reply, flags);
}
} // namespace android
