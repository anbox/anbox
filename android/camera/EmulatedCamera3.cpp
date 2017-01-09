/*
 * Copyright (C) 2013 The Android Open Source Project
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

/**
 * Contains implementation of a class EmulatedCamera that encapsulates
 * functionality common to all version 3.0 emulated camera devices.  Instances
 * of this class (for each emulated camera) are created during the construction
 * of the EmulatedCameraFactory instance.  This class serves as an entry point
 * for all camera API calls that defined by camera3_device_ops_t API.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera3_Camera"
#include <cutils/log.h>

#include "EmulatedCamera3.h"
#include "system/camera_metadata.h"

namespace android {

/**
 * Constructs EmulatedCamera3 instance.
 * Param:
 *  cameraId - Zero based camera identifier, which is an index of the camera
 *      instance in camera factory's array.
 *  module - Emulated camera HAL module descriptor.
 */
EmulatedCamera3::EmulatedCamera3(int cameraId,
        struct hw_module_t* module):
        EmulatedBaseCamera(cameraId,
                CAMERA_DEVICE_API_VERSION_3_3,
                &common,
                module),
        mStatus(STATUS_ERROR)
{
    common.close = EmulatedCamera3::close;
    ops = &sDeviceOps;

    mCallbackOps = NULL;

}

/* Destructs EmulatedCamera3 instance. */
EmulatedCamera3::~EmulatedCamera3() {
}

/****************************************************************************
 * Abstract API
 ***************************************************************************/

/****************************************************************************
 * Public API
 ***************************************************************************/

status_t EmulatedCamera3::Initialize() {
    ALOGV("%s", __FUNCTION__);

    mStatus = STATUS_CLOSED;
    return NO_ERROR;
}

/****************************************************************************
 * Camera API implementation
 ***************************************************************************/

status_t EmulatedCamera3::connectCamera(hw_device_t** device) {
    ALOGV("%s", __FUNCTION__);
    if (device == NULL) return BAD_VALUE;

    if (mStatus != STATUS_CLOSED) {
        ALOGE("%s: Trying to open a camera in state %d!",
                __FUNCTION__, mStatus);
        return INVALID_OPERATION;
    }

    *device = &common;
    mStatus = STATUS_OPEN;
    return NO_ERROR;
}

status_t EmulatedCamera3::closeCamera() {
    mStatus = STATUS_CLOSED;
    return NO_ERROR;
}

status_t EmulatedCamera3::getCameraInfo(struct camera_info* info) {
    return EmulatedBaseCamera::getCameraInfo(info);
}

/****************************************************************************
 * Camera Device API implementation.
 * These methods are called from the camera API callback routines.
 ***************************************************************************/

status_t EmulatedCamera3::initializeDevice(
        const camera3_callback_ops *callbackOps) {
    if (callbackOps == NULL) {
        ALOGE("%s: NULL callback ops provided to HAL!",
                __FUNCTION__);
        return BAD_VALUE;
    }

    if (mStatus != STATUS_OPEN) {
        ALOGE("%s: Trying to initialize a camera in state %d!",
                __FUNCTION__, mStatus);
        return INVALID_OPERATION;
    }

    mCallbackOps = callbackOps;
    mStatus = STATUS_READY;

    return NO_ERROR;
}

status_t EmulatedCamera3::configureStreams(
        camera3_stream_configuration *streamList) {
    ALOGE("%s: Not implemented", __FUNCTION__);
    return INVALID_OPERATION;
}

status_t EmulatedCamera3::registerStreamBuffers(
        const camera3_stream_buffer_set *bufferSet) {
    ALOGE("%s: Not implemented", __FUNCTION__);
    return INVALID_OPERATION;
}

const camera_metadata_t* EmulatedCamera3::constructDefaultRequestSettings(
        int type) {
    ALOGE("%s: Not implemented", __FUNCTION__);
    return NULL;
}

status_t EmulatedCamera3::processCaptureRequest(
        camera3_capture_request *request) {
    ALOGE("%s: Not implemented", __FUNCTION__);
    return INVALID_OPERATION;
}

status_t EmulatedCamera3::flush() {
    ALOGE("%s: Not implemented", __FUNCTION__);
    return INVALID_OPERATION;
}

/** Debug methods */

void EmulatedCamera3::dump(int fd) {
    ALOGE("%s: Not implemented", __FUNCTION__);
    return;
}

/****************************************************************************
 * Protected API. Callbacks to the framework.
 ***************************************************************************/

void EmulatedCamera3::sendCaptureResult(camera3_capture_result_t *result) {
    mCallbackOps->process_capture_result(mCallbackOps, result);
}

void EmulatedCamera3::sendNotify(camera3_notify_msg_t *msg) {
    mCallbackOps->notify(mCallbackOps, msg);
}

/****************************************************************************
 * Private API.
 ***************************************************************************/

/****************************************************************************
 * Camera API callbacks as defined by camera3_device_ops structure.  See
 * hardware/libhardware/include/hardware/camera3.h for information on each
 * of these callbacks. Implemented in this class, these callbacks simply
 * dispatch the call into an instance of EmulatedCamera3 class defined by the
 * 'camera_device3' parameter, or set a member value in the same.
 ***************************************************************************/

EmulatedCamera3* getInstance(const camera3_device_t *d) {
    const EmulatedCamera3* cec = static_cast<const EmulatedCamera3*>(d);
    return const_cast<EmulatedCamera3*>(cec);
}

int EmulatedCamera3::initialize(const struct camera3_device *d,
        const camera3_callback_ops_t *callback_ops) {
    EmulatedCamera3* ec = getInstance(d);
    return ec->initializeDevice(callback_ops);
}

int EmulatedCamera3::configure_streams(const struct camera3_device *d,
        camera3_stream_configuration_t *stream_list) {
    EmulatedCamera3* ec = getInstance(d);
    return ec->configureStreams(stream_list);
}

int EmulatedCamera3::register_stream_buffers(
        const struct camera3_device *d,
        const camera3_stream_buffer_set_t *buffer_set) {
    EmulatedCamera3* ec = getInstance(d);
    return ec->registerStreamBuffers(buffer_set);
}

int EmulatedCamera3::process_capture_request(
        const struct camera3_device *d,
        camera3_capture_request_t *request) {
    EmulatedCamera3* ec = getInstance(d);
    return ec->processCaptureRequest(request);
}

const camera_metadata_t* EmulatedCamera3::construct_default_request_settings(
        const camera3_device_t *d, int type) {
    EmulatedCamera3* ec = getInstance(d);
    return ec->constructDefaultRequestSettings(type);
}

void EmulatedCamera3::dump(const camera3_device_t *d, int fd) {
    EmulatedCamera3* ec = getInstance(d);
    ec->dump(fd);
}

int EmulatedCamera3::flush(const camera3_device_t *d) {
    EmulatedCamera3* ec = getInstance(d);
    return ec->flush();
}

int EmulatedCamera3::close(struct hw_device_t* device) {
    EmulatedCamera3* ec =
            static_cast<EmulatedCamera3*>(
                reinterpret_cast<camera3_device_t*>(device) );
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera3 device", __FUNCTION__);
        return BAD_VALUE;
    }
    return ec->closeCamera();
}

camera3_device_ops_t EmulatedCamera3::sDeviceOps = {
    EmulatedCamera3::initialize,
    EmulatedCamera3::configure_streams,
    /* DEPRECATED: register_stream_buffers */ nullptr,
    EmulatedCamera3::construct_default_request_settings,
    EmulatedCamera3::process_capture_request,
    /* DEPRECATED: get_metadata_vendor_tag_ops */ nullptr,
    EmulatedCamera3::dump,
    EmulatedCamera3::flush
};

const char* EmulatedCamera3::sAvailableCapabilitiesStrings[NUM_CAPABILITIES] = {
    "BACKWARD_COMPATIBLE",
    "MANUAL_SENSOR",
    "MANUAL_POST_PROCESSING",
    "RAW",
    "PRIVATE_REPROCESSING",
    "READ_SENSOR_SETTINGS",
    "BURST_CAPTURE",
    "YUV_REPROCESSING",
    "DEPTH_OUTPUT",
    "CONSTRAINED_HIGH_SPEED_VIDEO",
    "FULL_LEVEL"
};

}; /* namespace android */
