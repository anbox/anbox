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
 * Contains implementation of a class EmulatedQemuCamera that encapsulates
 * functionality of an emulated camera connected to the host.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera_QemuCamera"
#include <cutils/log.h>
#include "EmulatedQemuCamera.h"
#include "EmulatedCameraFactory.h"

namespace android {

EmulatedQemuCamera::EmulatedQemuCamera(int cameraId, struct hw_module_t* module)
        : EmulatedCamera(cameraId, module),
          mQemuCameraDevice(this)
{
}

EmulatedQemuCamera::~EmulatedQemuCamera()
{
}

/****************************************************************************
 * EmulatedCamera virtual overrides.
 ***************************************************************************/

status_t EmulatedQemuCamera::Initialize(const char* device_name,
                                        const char* frame_dims,
                                        const char* facing_dir)
{
    ALOGV("%s:\n   Name=%s\n   Facing '%s'\n   Dimensions=%s",
         __FUNCTION__, device_name, facing_dir, frame_dims);
    /* Save dimensions. */
    mFrameDims = frame_dims;

    /* Initialize camera device. */
    status_t res = mQemuCameraDevice.Initialize(device_name);
    if (res != NO_ERROR) {
        return res;
    }

    /* Initialize base class. */
    res = EmulatedCamera::Initialize();
    if (res != NO_ERROR) {
        return res;
    }

    /*
     * Set customizable parameters.
     */

    mParameters.set(EmulatedCamera::FACING_KEY, facing_dir);
    mParameters.set(EmulatedCamera::ORIENTATION_KEY,
                    gEmulatedCameraFactory.getQemuCameraOrientation());
    mParameters.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES, frame_dims);
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, frame_dims);

    /*
     * Use first dimension reported by the device to set current preview and
     * picture sizes.
     */

    char first_dim[128];
    /* Dimensions are separated with ',' */
    const char* c = strchr(frame_dims, ',');
    if (c == NULL) {
        strncpy(first_dim, frame_dims, sizeof(first_dim));
        first_dim[sizeof(first_dim)-1] = '\0';
    } else if (static_cast<size_t>(c - frame_dims) < sizeof(first_dim)) {
        memcpy(first_dim, frame_dims, c - frame_dims);
        first_dim[c - frame_dims] = '\0';
    } else {
        memcpy(first_dim, frame_dims, sizeof(first_dim));
        first_dim[sizeof(first_dim)-1] = '\0';
    }

    /* Width and height are separated with 'x' */
    char* sep = strchr(first_dim, 'x');
    if (sep == NULL) {
        ALOGE("%s: Invalid first dimension format in %s",
             __FUNCTION__, frame_dims);
        return EINVAL;
    }

    *sep = '\0';
    const int x = atoi(first_dim);
    const int y = atoi(sep + 1);
    mParameters.setPreviewSize(x, y);
    mParameters.setPictureSize(x, y);

    ALOGV("%s: Qemu camera %s is initialized. Current frame is %dx%d",
         __FUNCTION__, device_name, x, y);

    return NO_ERROR;
}

EmulatedCameraDevice* EmulatedQemuCamera::getCameraDevice()
{
    return &mQemuCameraDevice;
}

};  /* namespace android */
