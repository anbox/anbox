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

#ifndef ANBOX_ANDROID_PLATFORM_SERVICE_INTERFACE_H_
#define ANBOX_ANDROID_PLATFORM_SERVICE_INTERFACE_H_

#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <utils/String16.h>
#include <utils/StrongPointer.h>

#include <cstdint>

namespace android {
class IPlatformService : public IInterface {
public:
    DECLARE_META_INTERFACE(PlatformService);

    enum {
        BOOT_FINISHED = IBinder::FIRST_CALL_TRANSACTION,
    };

    virtual status_t boot_finished() = 0;
};

class BpPlatformService : public BpInterface<IPlatformService> {
public:
    BpPlatformService(const sp<IBinder> &binder);

    status_t boot_finished() override;
};

class BnPlatformService : public BnInterface<IPlatformService> {
public:
    virtual status_t onTransact(uint32_t code, const Parcel &data,
                                Parcel *reply, uint32_t flags);
};
} // namespace android
#endif
