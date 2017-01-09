/*
 * Copyright (C) 2012 The Android Open Source Project
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
 * Contains implementation of a class EmulatedBaseCamera that encapsulates
 * functionality common to all emulated camera device versions ("fake",
 * "webcam", "video file", "cam2.0" etc.).  Instances of this class (for each
 * emulated camera) are created during the construction of the
 * EmulatedCameraFactory instance.  This class serves as an entry point for all
 * camera API calls that are common across all versions of the
 * camera_device_t/camera_module_t structures.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera_BaseCamera"
#include <cutils/log.h>

#include "EmulatedBaseCamera.h"

namespace android {

EmulatedBaseCamera::EmulatedBaseCamera(int cameraId,
        uint32_t cameraVersion,
        struct hw_device_t* device,
        struct hw_module_t* module)
        : mCameraInfo(NULL),
          mCameraID(cameraId),
          mCameraDeviceVersion(cameraVersion)
{
    /*
     * Initialize camera_device descriptor for this object.
     */

    /* Common header */
    device->tag = HARDWARE_DEVICE_TAG;
    device->version = cameraVersion;
    device->module = module;
    device->close = NULL; // Must be filled in by child implementation
}

EmulatedBaseCamera::~EmulatedBaseCamera()
{
}

status_t EmulatedBaseCamera::getCameraInfo(struct camera_info* info)
{
    ALOGV("%s", __FUNCTION__);

    info->device_version = mCameraDeviceVersion;
    if (mCameraDeviceVersion >= HARDWARE_DEVICE_API_VERSION(2, 0)) {
        info->static_camera_characteristics = mCameraInfo;
    } else {
        info->static_camera_characteristics = (camera_metadata_t*)0xcafef00d;
    }

    return NO_ERROR;
}

status_t EmulatedBaseCamera::plugCamera() {
    ALOGE("%s: not supported", __FUNCTION__);
    return INVALID_OPERATION;
}

status_t EmulatedBaseCamera::unplugCamera() {
    ALOGE("%s: not supported", __FUNCTION__);
    return INVALID_OPERATION;
}

camera_device_status_t EmulatedBaseCamera::getHotplugStatus() {
    return CAMERA_DEVICE_STATUS_PRESENT;
}




} /* namespace android */
