/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Contains implementation of a class EmulatedQemuCameraDevice that encapsulates
 * an emulated camera device connected to the host.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera_QemuDevice"
#include <cutils/log.h>
#include "EmulatedQemuCamera.h"
#include "EmulatedQemuCameraDevice.h"

namespace android {

EmulatedQemuCameraDevice::EmulatedQemuCameraDevice(EmulatedQemuCamera* camera_hal)
    : EmulatedCameraDevice(camera_hal),
      mQemuClient(),
      mPreviewFrame(NULL)
{
}

EmulatedQemuCameraDevice::~EmulatedQemuCameraDevice()
{
    if (mPreviewFrame != NULL) {
        delete[] mPreviewFrame;
    }
}

/****************************************************************************
 * Public API
 ***************************************************************************/

status_t EmulatedQemuCameraDevice::Initialize(const char* device_name)
{
    /* Connect to the service. */
    char connect_str[256];
    snprintf(connect_str, sizeof(connect_str), "name=%s", device_name);
    status_t res = mQemuClient.connectClient(connect_str);
    if (res != NO_ERROR) {
        return res;
    }

    /* Initialize base class. */
    res = EmulatedCameraDevice::Initialize();
    if (res == NO_ERROR) {
        ALOGV("%s: Connected to the emulated camera service '%s'",
             __FUNCTION__, device_name);
        mDeviceName = device_name;
    } else {
        mQemuClient.queryDisconnect();
    }

    return res;
}

/****************************************************************************
 * Emulated camera device abstract interface implementation.
 ***************************************************************************/

status_t EmulatedQemuCameraDevice::connectDevice()
{
    ALOGV("%s", __FUNCTION__);

    Mutex::Autolock locker(&mObjectLock);
    if (!isInitialized()) {
        ALOGE("%s: Qemu camera device is not initialized.", __FUNCTION__);
        return EINVAL;
    }
    if (isConnected()) {
        ALOGW("%s: Qemu camera device '%s' is already connected.",
             __FUNCTION__, (const char*)mDeviceName);
        return NO_ERROR;
    }

    /* Connect to the camera device via emulator. */
    const status_t res = mQemuClient.queryConnect();
    if (res == NO_ERROR) {
        ALOGV("%s: Connected to device '%s'",
             __FUNCTION__, (const char*)mDeviceName);
        mState = ECDS_CONNECTED;
    } else {
        ALOGE("%s: Connection to device '%s' failed",
             __FUNCTION__, (const char*)mDeviceName);
    }

    return res;
}

status_t EmulatedQemuCameraDevice::disconnectDevice()
{
    ALOGV("%s", __FUNCTION__);

    Mutex::Autolock locker(&mObjectLock);
    if (!isConnected()) {
        ALOGW("%s: Qemu camera device '%s' is already disconnected.",
             __FUNCTION__, (const char*)mDeviceName);
        return NO_ERROR;
    }
    if (isStarted()) {
        ALOGE("%s: Cannot disconnect from the started device '%s.",
             __FUNCTION__, (const char*)mDeviceName);
        return EINVAL;
    }

    /* Disconnect from the camera device via emulator. */
    const status_t res = mQemuClient.queryDisconnect();
    if (res == NO_ERROR) {
        ALOGV("%s: Disonnected from device '%s'",
             __FUNCTION__, (const char*)mDeviceName);
        mState = ECDS_INITIALIZED;
    } else {
        ALOGE("%s: Disconnection from device '%s' failed",
             __FUNCTION__, (const char*)mDeviceName);
    }

    return res;
}

status_t EmulatedQemuCameraDevice::startDevice(int width,
                                               int height,
                                               uint32_t pix_fmt)
{
    ALOGV("%s", __FUNCTION__);

    Mutex::Autolock locker(&mObjectLock);
    if (!isConnected()) {
        ALOGE("%s: Qemu camera device '%s' is not connected.",
             __FUNCTION__, (const char*)mDeviceName);
        return EINVAL;
    }
    if (isStarted()) {
        ALOGW("%s: Qemu camera device '%s' is already started.",
             __FUNCTION__, (const char*)mDeviceName);
        return NO_ERROR;
    }

    status_t res = EmulatedCameraDevice::commonStartDevice(width, height, pix_fmt);
    if (res != NO_ERROR) {
        ALOGE("%s: commonStartDevice failed", __FUNCTION__);
        return res;
    }

    /* Allocate preview frame buffer. */
    /* TODO: Watch out for preview format changes! At this point we implement
     * RGB32 only.*/
    mPreviewFrame = new uint32_t[mTotalPixels];
    if (mPreviewFrame == NULL) {
        ALOGE("%s: Unable to allocate %d bytes for preview frame",
             __FUNCTION__, mTotalPixels);
        return ENOMEM;
    }

    /* Start the actual camera device. */
    res = mQemuClient.queryStart(mPixelFormat, mFrameWidth, mFrameHeight);
    if (res == NO_ERROR) {
        ALOGV("%s: Qemu camera device '%s' is started for %.4s[%dx%d] frames",
             __FUNCTION__, (const char*)mDeviceName,
             reinterpret_cast<const char*>(&mPixelFormat),
             mFrameWidth, mFrameHeight);
        mState = ECDS_STARTED;
    } else {
        ALOGE("%s: Unable to start device '%s' for %.4s[%dx%d] frames",
             __FUNCTION__, (const char*)mDeviceName,
             reinterpret_cast<const char*>(&pix_fmt), width, height);
    }

    return res;
}

status_t EmulatedQemuCameraDevice::stopDevice()
{
    ALOGV("%s", __FUNCTION__);

    Mutex::Autolock locker(&mObjectLock);
    if (!isStarted()) {
        ALOGW("%s: Qemu camera device '%s' is not started.",
             __FUNCTION__, (const char*)mDeviceName);
        return NO_ERROR;
    }

    /* Stop the actual camera device. */
    status_t res = mQemuClient.queryStop();
    if (res == NO_ERROR) {
        if (mPreviewFrame == NULL) {
            delete[] mPreviewFrame;
            mPreviewFrame = NULL;
        }
        EmulatedCameraDevice::commonStopDevice();
        mState = ECDS_CONNECTED;
        ALOGV("%s: Qemu camera device '%s' is stopped",
             __FUNCTION__, (const char*)mDeviceName);
    } else {
        ALOGE("%s: Unable to stop device '%s'",
             __FUNCTION__, (const char*)mDeviceName);
    }

    return res;
}

/****************************************************************************
 * EmulatedCameraDevice virtual overrides
 ***************************************************************************/

status_t EmulatedQemuCameraDevice::getCurrentPreviewFrame(void* buffer)
{
    ALOGW_IF(mPreviewFrame == NULL, "%s: No preview frame", __FUNCTION__);
    if (mPreviewFrame != NULL) {
        memcpy(buffer, mPreviewFrame, mTotalPixels * 4);
        return 0;
    } else {
        return EmulatedCameraDevice::getCurrentPreviewFrame(buffer);
    }
}

/****************************************************************************
 * Worker thread management overrides.
 ***************************************************************************/

bool EmulatedQemuCameraDevice::inWorkerThread()
{
    /* Wait till FPS timeout expires, or thread exit message is received. */
    WorkerThread::SelectRes res =
        getWorkerThread()->Select(-1, 1000000 / mEmulatedFPS);
    if (res == WorkerThread::EXIT_THREAD) {
        ALOGV("%s: Worker thread has been terminated.", __FUNCTION__);
        return false;
    }

    /* Query frames from the service. */
    status_t query_res = mQemuClient.queryFrame(mCurrentFrame, mPreviewFrame,
                                                 mFrameBufferSize,
                                                 mTotalPixels * 4,
                                                 mWhiteBalanceScale[0],
                                                 mWhiteBalanceScale[1],
                                                 mWhiteBalanceScale[2],
                                                 mExposureCompensation);
    if (query_res == NO_ERROR) {
        /* Timestamp the current frame, and notify the camera HAL. */
        mCurFrameTimestamp = systemTime(SYSTEM_TIME_MONOTONIC);
        mCameraHAL->onNextFrameAvailable(mCurrentFrame, mCurFrameTimestamp, this);
        return true;
    } else {
        ALOGE("%s: Unable to get current video frame: %s",
             __FUNCTION__, strerror(query_res));
        mCameraHAL->onCameraDeviceError(CAMERA_ERROR_SERVER_DIED);
        return false;
    }
}

}; /* namespace android */
