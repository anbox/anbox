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

#define LOG_TAG "Anboxd"

#include "android/service/activity_manager_interface.h"

namespace android {
BpActivityManager::BpActivityManager(const sp<IBinder> &binder) :
    BpInterface<IActivityManager>(binder) {
}

status_t BpActivityManager::setFocusedTask(int32_t id) {
    Parcel data, reply;
    data.writeInterfaceToken(IActivityManager::getInterfaceDescriptor());
    data.writeInt32(id);
    return remote()->transact(IActivityManager::SET_FOCUSED_TASK, data, &reply);
}

status_t BpActivityManager::removeTask(int32_t id) {
  Parcel data, reply;
  data.writeInterfaceToken(IActivityManager::getInterfaceDescriptor());
  data.writeInt32(id);
  return remote()->transact(IActivityManager::REMOVE_TASK, data, &reply);
}

status_t BpActivityManager::resizeTask(int32_t id, const anbox::graphics::Rect &rect, int resize_mode) {
  Parcel data, reply;
  data.writeInterfaceToken(IActivityManager::getInterfaceDescriptor());
  data.writeInt32(id);
  data.writeInt32(resize_mode);
  data.writeInt32(rect.left());
  data.writeInt32(rect.top());
  data.writeInt32(rect.right());
  data.writeInt32(rect.bottom());
  return remote()->transact(IActivityManager::RESIZE_TASK, data, &reply);
}

IMPLEMENT_META_INTERFACE(ActivityManager, "android.app.IActivityManager");
} // namespace android
