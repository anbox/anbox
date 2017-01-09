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
 * Contains implementation of a class EmulatedCameraFactory that manages cameras
 * available for emulation.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera_Factory"
#include <cutils/log.h>
#include <cutils/properties.h>
#include "EmulatedQemuCamera.h"
#include "EmulatedFakeCamera.h"
#include "EmulatedFakeCamera2.h"
#include "EmulatedFakeCamera3.h"
#include "EmulatedCameraHotplugThread.h"
#include "EmulatedCameraFactory.h"

extern camera_module_t HAL_MODULE_INFO_SYM;

/* A global instance of EmulatedCameraFactory is statically instantiated and
 * initialized when camera emulation HAL is loaded.
 */
android::EmulatedCameraFactory  gEmulatedCameraFactory;

namespace android {

EmulatedCameraFactory::EmulatedCameraFactory()
        : mQemuClient(),
          mEmulatedCameras(NULL),
          mEmulatedCameraNum(0),
          mFakeCameraNum(0),
          mConstructedOK(false),
          mCallbacks(NULL)
{
    status_t res;
    /* Connect to the factory service in the emulator, and create Qemu cameras. */
    if (mQemuClient.connectClient(NULL) == NO_ERROR) {
        /* Connection has succeeded. Create emulated cameras for each camera
         * device, reported by the service. */
        createQemuCameras();
    }

    if (isBackFakeCameraEmulationOn()) {
        /* Camera ID. */
        const int camera_id = mEmulatedCameraNum;
        /* Use fake camera to emulate back-facing camera. */
        mEmulatedCameraNum++;

        /* Make sure that array is allocated (in case there were no 'qemu'
         * cameras created. Note that we preallocate the array so it may contain
         * two fake cameras: one facing back, and another facing front. */
        if (mEmulatedCameras == NULL) {
            mEmulatedCameras = new EmulatedBaseCamera*[mEmulatedCameraNum + 1];
            if (mEmulatedCameras == NULL) {
                ALOGE("%s: Unable to allocate emulated camera array for %d entries",
                     __FUNCTION__, mEmulatedCameraNum);
                return;
            }
            memset(mEmulatedCameras, 0,
                    (mEmulatedCameraNum + 1) * sizeof(EmulatedBaseCamera*));
        }

        /* Create, and initialize the fake camera */
        switch (getBackCameraHalVersion()) {
            case 1:
                mEmulatedCameras[camera_id] =
                        new EmulatedFakeCamera(camera_id, true,
                                &HAL_MODULE_INFO_SYM.common);
                break;
            case 2:
                mEmulatedCameras[camera_id] =
                        new EmulatedFakeCamera2(camera_id, true,
                                &HAL_MODULE_INFO_SYM.common);
                break;
            case 3:
                mEmulatedCameras[camera_id] =
                        new EmulatedFakeCamera3(camera_id, true,
                                &HAL_MODULE_INFO_SYM.common);
                break;
            default:
                ALOGE("%s: Unknown back camera hal version requested: %d", __FUNCTION__,
                        getBackCameraHalVersion());
        }
        if (mEmulatedCameras[camera_id] != NULL) {
            ALOGV("%s: Back camera device version is %d", __FUNCTION__,
                    getBackCameraHalVersion());
            res = mEmulatedCameras[camera_id]->Initialize();
            if (res != NO_ERROR) {
                ALOGE("%s: Unable to intialize back camera %d: %s (%d)",
                        __FUNCTION__, camera_id, strerror(-res), res);
                delete mEmulatedCameras[camera_id];
                mEmulatedCameraNum--;
            }
        } else {
            mEmulatedCameraNum--;
            ALOGE("%s: Unable to instantiate fake camera class", __FUNCTION__);
        }
    }

    if (isFrontFakeCameraEmulationOn()) {
        /* Camera ID. */
        const int camera_id = mEmulatedCameraNum;
        /* Use fake camera to emulate front-facing camera. */
        mEmulatedCameraNum++;

        /* Make sure that array is allocated (in case there were no 'qemu'
         * cameras created. */
        if (mEmulatedCameras == NULL) {
            mEmulatedCameras = new EmulatedBaseCamera*[mEmulatedCameraNum];
            if (mEmulatedCameras == NULL) {
                ALOGE("%s: Unable to allocate emulated camera array for %d entries",
                     __FUNCTION__, mEmulatedCameraNum);
                return;
            }
            memset(mEmulatedCameras, 0,
                    mEmulatedCameraNum * sizeof(EmulatedBaseCamera*));
        }

        /* Create, and initialize the fake camera */
        switch (getFrontCameraHalVersion()) {
            case 1:
                mEmulatedCameras[camera_id] =
                        new EmulatedFakeCamera(camera_id, false,
                                &HAL_MODULE_INFO_SYM.common);
                break;
            case 2:
                mEmulatedCameras[camera_id] =
                        new EmulatedFakeCamera2(camera_id, false,
                                &HAL_MODULE_INFO_SYM.common);
                break;
            case 3:
                mEmulatedCameras[camera_id] =
                        new EmulatedFakeCamera3(camera_id, false,
                                &HAL_MODULE_INFO_SYM.common);
                break;
            default:
                ALOGE("%s: Unknown front camera hal version requested: %d",
                        __FUNCTION__,
                        getFrontCameraHalVersion());
        }
        if (mEmulatedCameras[camera_id] != NULL) {
            ALOGV("%s: Front camera device version is %d", __FUNCTION__,
                    getFrontCameraHalVersion());
            res = mEmulatedCameras[camera_id]->Initialize();
            if (res != NO_ERROR) {
                ALOGE("%s: Unable to intialize front camera %d: %s (%d)",
                        __FUNCTION__, camera_id, strerror(-res), res);
                delete mEmulatedCameras[camera_id];
                mEmulatedCameraNum--;
            }
        } else {
            mEmulatedCameraNum--;
            ALOGE("%s: Unable to instantiate fake camera class", __FUNCTION__);
        }
    }

    ALOGV("%d cameras are being emulated. %d of them are fake cameras.",
          mEmulatedCameraNum, mFakeCameraNum);

    /* Create hotplug thread */
    {
        Vector<int> cameraIdVector;
        for (int i = 0; i < mEmulatedCameraNum; ++i) {
            cameraIdVector.push_back(i);
        }
        mHotplugThread = new EmulatedCameraHotplugThread(&cameraIdVector[0],
                                                         mEmulatedCameraNum);
        mHotplugThread->run("EmulatedCameraHotplugThread");
    }

    mConstructedOK = true;
}

EmulatedCameraFactory::~EmulatedCameraFactory()
{
    if (mEmulatedCameras != NULL) {
        for (int n = 0; n < mEmulatedCameraNum; n++) {
            if (mEmulatedCameras[n] != NULL) {
                delete mEmulatedCameras[n];
            }
        }
        delete[] mEmulatedCameras;
    }

    if (mHotplugThread != NULL) {
        mHotplugThread->requestExit();
        mHotplugThread->join();
    }
}

/****************************************************************************
 * Camera HAL API handlers.
 *
 * Each handler simply verifies existence of an appropriate EmulatedBaseCamera
 * instance, and dispatches the call to that instance.
 *
 ***************************************************************************/

int EmulatedCameraFactory::cameraDeviceOpen(int camera_id, hw_device_t** device)
{
    ALOGV("%s: id = %d", __FUNCTION__, camera_id);

    *device = NULL;

    if (!isConstructedOK()) {
        ALOGE("%s: EmulatedCameraFactory has failed to initialize", __FUNCTION__);
        return -EINVAL;
    }

    if (camera_id < 0 || camera_id >= getEmulatedCameraNum()) {
        ALOGE("%s: Camera id %d is out of bounds (%d)",
             __FUNCTION__, camera_id, getEmulatedCameraNum());
        return -ENODEV;
    }

    return mEmulatedCameras[camera_id]->connectCamera(device);
}

int EmulatedCameraFactory::getCameraInfo(int camera_id, struct camera_info* info)
{
    ALOGV("%s: id = %d", __FUNCTION__, camera_id);

    if (!isConstructedOK()) {
        ALOGE("%s: EmulatedCameraFactory has failed to initialize", __FUNCTION__);
        return -EINVAL;
    }

    if (camera_id < 0 || camera_id >= getEmulatedCameraNum()) {
        ALOGE("%s: Camera id %d is out of bounds (%d)",
             __FUNCTION__, camera_id, getEmulatedCameraNum());
        return -ENODEV;
    }

    return mEmulatedCameras[camera_id]->getCameraInfo(info);
}

int EmulatedCameraFactory::setCallbacks(
        const camera_module_callbacks_t *callbacks)
{
    ALOGV("%s: callbacks = %p", __FUNCTION__, callbacks);

    mCallbacks = callbacks;

    return OK;
}

void EmulatedCameraFactory::getVendorTagOps(vendor_tag_ops_t* ops) {
    ALOGV("%s: ops = %p", __FUNCTION__, ops);

    // No vendor tags defined for emulator yet, so not touching ops
}

/****************************************************************************
 * Camera HAL API callbacks.
 ***************************************************************************/

int EmulatedCameraFactory::device_open(const hw_module_t* module,
                                       const char* name,
                                       hw_device_t** device)
{
    /*
     * Simply verify the parameters, and dispatch the call inside the
     * EmulatedCameraFactory instance.
     */

    if (module != &HAL_MODULE_INFO_SYM.common) {
        ALOGE("%s: Invalid module %p expected %p",
             __FUNCTION__, module, &HAL_MODULE_INFO_SYM.common);
        return -EINVAL;
    }
    if (name == NULL) {
        ALOGE("%s: NULL name is not expected here", __FUNCTION__);
        return -EINVAL;
    }

    return gEmulatedCameraFactory.cameraDeviceOpen(atoi(name), device);
}

int EmulatedCameraFactory::get_number_of_cameras(void)
{
    return gEmulatedCameraFactory.getEmulatedCameraNum();
}

int EmulatedCameraFactory::get_camera_info(int camera_id,
                                           struct camera_info* info)
{
    return gEmulatedCameraFactory.getCameraInfo(camera_id, info);
}

int EmulatedCameraFactory::set_callbacks(
        const camera_module_callbacks_t *callbacks)
{
    return gEmulatedCameraFactory.setCallbacks(callbacks);
}

void EmulatedCameraFactory::get_vendor_tag_ops(vendor_tag_ops_t* ops)
{
    gEmulatedCameraFactory.getVendorTagOps(ops);
}

int EmulatedCameraFactory::open_legacy(const struct hw_module_t* module,
        const char* id, uint32_t halVersion, struct hw_device_t** device) {
    // Not supporting legacy open
    return -ENOSYS;
}

/********************************************************************************
 * Internal API
 *******************************************************************************/

/*
 * Camera information tokens passed in response to the "list" factory query.
 */

/* Device name token. */
static const char lListNameToken[]    = "name=";
/* Frame dimensions token. */
static const char lListDimsToken[]    = "framedims=";
/* Facing direction token. */
static const char lListDirToken[]     = "dir=";

void EmulatedCameraFactory::createQemuCameras()
{
    /* Obtain camera list. */
    char* camera_list = NULL;
    status_t res = mQemuClient.listCameras(&camera_list);
    /* Empty list, or list containing just an EOL means that there were no
     * connected cameras found. */
    if (res != NO_ERROR || camera_list == NULL || *camera_list == '\0' ||
        *camera_list == '\n') {
        if (camera_list != NULL) {
            free(camera_list);
        }
        return;
    }

    /*
     * Calculate number of connected cameras. Number of EOLs in the camera list
     * is the number of the connected cameras.
     */

    int num = 0;
    const char* eol = strchr(camera_list, '\n');
    while (eol != NULL) {
        num++;
        eol = strchr(eol + 1, '\n');
    }

    /* Allocate the array for emulated camera instances. Note that we allocate
     * two more entries for back and front fake camera emulation. */
    mEmulatedCameras = new EmulatedBaseCamera*[num + 2];
    if (mEmulatedCameras == NULL) {
        ALOGE("%s: Unable to allocate emulated camera array for %d entries",
             __FUNCTION__, num + 1);
        free(camera_list);
        return;
    }
    memset(mEmulatedCameras, 0, sizeof(EmulatedBaseCamera*) * (num + 1));

    /*
     * Iterate the list, creating, and initializin emulated qemu cameras for each
     * entry (line) in the list.
     */

    int index = 0;
    char* cur_entry = camera_list;
    while (cur_entry != NULL && *cur_entry != '\0' && index < num) {
        /* Find the end of the current camera entry, and terminate it with zero
         * for simpler string manipulation. */
        char* next_entry = strchr(cur_entry, '\n');
        if (next_entry != NULL) {
            *next_entry = '\0';
            next_entry++;   // Start of the next entry.
        }

        /* Find 'name', 'framedims', and 'dir' tokens that are required here. */
        char* name_start = strstr(cur_entry, lListNameToken);
        char* dim_start = strstr(cur_entry, lListDimsToken);
        char* dir_start = strstr(cur_entry, lListDirToken);
        if (name_start != NULL && dim_start != NULL && dir_start != NULL) {
            /* Advance to the token values. */
            name_start += strlen(lListNameToken);
            dim_start += strlen(lListDimsToken);
            dir_start += strlen(lListDirToken);

            /* Terminate token values with zero. */
            char* s = strchr(name_start, ' ');
            if (s != NULL) {
                *s = '\0';
            }
            s = strchr(dim_start, ' ');
            if (s != NULL) {
                *s = '\0';
            }
            s = strchr(dir_start, ' ');
            if (s != NULL) {
                *s = '\0';
            }

            /* Create and initialize qemu camera. */
            EmulatedQemuCamera* qemu_cam =
                new EmulatedQemuCamera(index, &HAL_MODULE_INFO_SYM.common);
            if (NULL != qemu_cam) {
                res = qemu_cam->Initialize(name_start, dim_start, dir_start);
                if (res == NO_ERROR) {
                    mEmulatedCameras[index] = qemu_cam;
                    index++;
                } else {
                    delete qemu_cam;
                }
            } else {
                ALOGE("%s: Unable to instantiate EmulatedQemuCamera",
                     __FUNCTION__);
            }
        } else {
            ALOGW("%s: Bad camera information: %s", __FUNCTION__, cur_entry);
        }

        cur_entry = next_entry;
    }

    mEmulatedCameraNum = index;
}

bool EmulatedCameraFactory::isBackFakeCameraEmulationOn()
{
    /* Defined by 'qemu.sf.fake_camera' boot property: if property exist, and
     * is set to 'both', or 'back', then fake camera is used to emulate back
     * camera. */
    char prop[PROPERTY_VALUE_MAX];
    if ((property_get("qemu.sf.fake_camera", prop, NULL) > 0) &&
        (!strcmp(prop, "both") || !strcmp(prop, "back"))) {
        return true;
    } else {
        return false;
    }
}

int EmulatedCameraFactory::getBackCameraHalVersion()
{
    /* Defined by 'qemu.sf.back_camera_hal_version' boot property: if the
     * property doesn't exist, it is assumed to be 1. */
    char prop[PROPERTY_VALUE_MAX];
    if (property_get("qemu.sf.back_camera_hal", prop, NULL) > 0) {
        char *prop_end = prop;
        int val = strtol(prop, &prop_end, 10);
        if (*prop_end == '\0') {
            return val;
        }
        // Badly formatted property, should just be a number
        ALOGE("qemu.sf.back_camera_hal is not a number: %s", prop);
    }
    return 1;
}

bool EmulatedCameraFactory::isFrontFakeCameraEmulationOn()
{
    /* Defined by 'qemu.sf.fake_camera' boot property: if property exist, and
     * is set to 'both', or 'front', then fake camera is used to emulate front
     * camera. */
    char prop[PROPERTY_VALUE_MAX];
    if ((property_get("qemu.sf.fake_camera", prop, NULL) > 0) &&
        (!strcmp(prop, "both") || !strcmp(prop, "front"))) {
        return true;
    } else {
        return false;
    }
}

int EmulatedCameraFactory::getFrontCameraHalVersion()
{
    /* Defined by 'qemu.sf.front_camera_hal_version' boot property: if the
     * property doesn't exist, it is assumed to be 1. */
    char prop[PROPERTY_VALUE_MAX];
    if (property_get("qemu.sf.front_camera_hal", prop, NULL) > 0) {
        char *prop_end = prop;
        int val = strtol(prop, &prop_end, 10);
        if (*prop_end == '\0') {
            return val;
        }
        // Badly formatted property, should just be a number
        ALOGE("qemu.sf.front_camera_hal is not a number: %s", prop);
    }
    return 1;
}

void EmulatedCameraFactory::onStatusChanged(int cameraId, int newStatus) {

    EmulatedBaseCamera *cam = mEmulatedCameras[cameraId];
    if (!cam) {
        ALOGE("%s: Invalid camera ID %d", __FUNCTION__, cameraId);
        return;
    }

    /**
     * (Order is important)
     * Send the callback first to framework, THEN close the camera.
     */

    if (newStatus == cam->getHotplugStatus()) {
        ALOGW("%s: Ignoring transition to the same status", __FUNCTION__);
        return;
    }

    const camera_module_callbacks_t* cb = mCallbacks;
    if (cb != NULL && cb->camera_device_status_change != NULL) {
        cb->camera_device_status_change(cb, cameraId, newStatus);
    }

    if (newStatus == CAMERA_DEVICE_STATUS_NOT_PRESENT) {
        cam->unplugCamera();
    } else if (newStatus == CAMERA_DEVICE_STATUS_PRESENT) {
        cam->plugCamera();
    }

}

/********************************************************************************
 * Initializer for the static member structure.
 *******************************************************************************/

/* Entry point for camera HAL API. */
struct hw_module_methods_t EmulatedCameraFactory::mCameraModuleMethods = {
    open: EmulatedCameraFactory::device_open
};

}; /* namespace android */
