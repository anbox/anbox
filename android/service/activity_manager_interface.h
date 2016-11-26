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

#ifndef ANBOX_ANDROID_ACTIVITY_MANAGER_INTERFACE_H_
#define ANBOX_ANDROID_ACTIVITY_MANAGER_INTERFACE_H_

#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <utils/String16.h>
#include <utils/StrongPointer.h>

#include <cstdint>

namespace android {
class IActivityManager : public IInterface {
public:
    DECLARE_META_INTERFACE(ActivityManager);

    enum {
        // This needs to stay synchronized with frameworks/base/core/java/android/app/IActivityManager.java
        SET_FOCUSED_TASK = IBinder::FIRST_CALL_TRANSACTION + 130,
    };

    virtual status_t setFocusedTask(int32_t id) = 0;
};

class BpActivityManager : public BpInterface<IActivityManager> {
public:
    BpActivityManager(const sp<IBinder> &binder);

    status_t setFocusedTask(int32_t id) override;
};
} // namespace android
#endif
