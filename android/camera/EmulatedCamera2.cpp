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
 * Contains implementation of a class EmulatedCamera that encapsulates
 * functionality common to all version 2.0 emulated camera devices.  Instances
 * of this class (for each emulated camera) are created during the construction
 * of the EmulatedCameraFactory instance.  This class serves as an entry point
 * for all camera API calls that defined by camera2_device_ops_t API.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera2_Camera"
#include <cutils/log.h>

#include "EmulatedCamera2.h"
#include "system/camera_metadata.h"

namespace android {

/* Constructs EmulatedCamera2 instance.
 * Param:
 *  cameraId - Zero based camera identifier, which is an index of the camera
 *      instance in camera factory's array.
 *  module - Emulated camera HAL module descriptor.
 */
EmulatedCamera2::EmulatedCamera2(int cameraId,
        struct hw_module_t* module):
        EmulatedBaseCamera(cameraId,
                CAMERA_DEVICE_API_VERSION_2_0,
                &common,
                module)
{
    common.close = EmulatedCamera2::close;
    ops = &sDeviceOps;
    priv = this;

    mNotifyCb = NULL;

    mRequestQueueSrc = NULL;
    mFrameQueueDst = NULL;

    mVendorTagOps.get_camera_vendor_section_name =
            EmulatedCamera2::get_camera_vendor_section_name;
    mVendorTagOps.get_camera_vendor_tag_name =
            EmulatedCamera2::get_camera_vendor_tag_name;
    mVendorTagOps.get_camera_vendor_tag_type =
            EmulatedCamera2::get_camera_vendor_tag_type;
    mVendorTagOps.parent = this;

    mStatusPresent = true;
}

/* Destructs EmulatedCamera2 instance. */
EmulatedCamera2::~EmulatedCamera2() {
}

/****************************************************************************
 * Abstract API
 ***************************************************************************/

/****************************************************************************
 * Public API
 ***************************************************************************/

status_t EmulatedCamera2::Initialize() {
    return NO_ERROR;
}

/****************************************************************************
 * Camera API implementation
 ***************************************************************************/

status_t EmulatedCamera2::connectCamera(hw_device_t** device) {
    *device = &common;
    return NO_ERROR;
}

status_t EmulatedCamera2::closeCamera() {
    return NO_ERROR;
}

status_t EmulatedCamera2::getCameraInfo(struct camera_info* info) {
    return EmulatedBaseCamera::getCameraInfo(info);
}

/****************************************************************************
 * Camera Device API implementation.
 * These methods are called from the camera API callback routines.
 ***************************************************************************/

/** Request input queue */

int EmulatedCamera2::requestQueueNotify() {
    return INVALID_OPERATION;
}

/** Count of requests in flight */
int EmulatedCamera2::getInProgressCount() {
    return INVALID_OPERATION;
}

/** Cancel all captures in flight */
int EmulatedCamera2::flushCapturesInProgress() {
    return INVALID_OPERATION;
}

/** Construct a default request for a given use case */
int EmulatedCamera2::constructDefaultRequest(
        int request_template,
        camera_metadata_t **request) {
    return INVALID_OPERATION;
}

/** Output stream creation and management */

int EmulatedCamera2::allocateStream(
        uint32_t width,
        uint32_t height,
        int format,
        const camera2_stream_ops_t *stream_ops,
        uint32_t *stream_id,
        uint32_t *format_actual,
        uint32_t *usage,
        uint32_t *max_buffers) {
    return INVALID_OPERATION;
}

int EmulatedCamera2::registerStreamBuffers(
        uint32_t stream_id,
        int num_buffers,
        buffer_handle_t *buffers) {
    return INVALID_OPERATION;
}


int EmulatedCamera2::releaseStream(uint32_t stream_id) {
    return INVALID_OPERATION;
}

/** Reprocessing input stream management */

int EmulatedCamera2::allocateReprocessStream(
        uint32_t width,
        uint32_t height,
        uint32_t format,
        const camera2_stream_in_ops_t *reprocess_stream_ops,
        uint32_t *stream_id,
        uint32_t *consumer_usage,
        uint32_t *max_buffers) {
    return INVALID_OPERATION;
}

int EmulatedCamera2::allocateReprocessStreamFromStream(
        uint32_t output_stream_id,
        const camera2_stream_in_ops_t *reprocess_stream_ops,
        uint32_t *stream_id) {
    return INVALID_OPERATION;
}

int EmulatedCamera2::releaseReprocessStream(uint32_t stream_id) {
    return INVALID_OPERATION;
}

/** 3A triggering */

int EmulatedCamera2::triggerAction(uint32_t trigger_id,
                                   int ext1, int ext2) {
    return INVALID_OPERATION;
}

/** Custom tag query methods */

const char* EmulatedCamera2::getVendorSectionName(uint32_t tag) {
    return NULL;
}

const char* EmulatedCamera2::getVendorTagName(uint32_t tag) {
    return NULL;
}

int EmulatedCamera2::getVendorTagType(uint32_t tag) {
    return -1;
}

/** Debug methods */

int EmulatedCamera2::dump(int fd) {
    return INVALID_OPERATION;
}

/****************************************************************************
 * Private API.
 ***************************************************************************/

/****************************************************************************
 * Camera API callbacks as defined by camera2_device_ops structure.  See
 * hardware/libhardware/include/hardware/camera2.h for information on each
 * of these callbacks. Implemented in this class, these callbacks simply
 * dispatch the call into an instance of EmulatedCamera2 class defined by the
 * 'camera_device2' parameter, or set a member value in the same.
 ***************************************************************************/

EmulatedCamera2* getInstance(const camera2_device_t *d) {
    const EmulatedCamera2* cec = static_cast<const EmulatedCamera2*>(d);
    return const_cast<EmulatedCamera2*>(cec);
}

int EmulatedCamera2::set_request_queue_src_ops(const camera2_device_t *d,
        const camera2_request_queue_src_ops *queue_src_ops) {
    EmulatedCamera2* ec = getInstance(d);
    ec->mRequestQueueSrc = queue_src_ops;
    return NO_ERROR;
}

int EmulatedCamera2::notify_request_queue_not_empty(const camera2_device_t *d) {
    EmulatedCamera2* ec = getInstance(d);
    return ec->requestQueueNotify();
}

int EmulatedCamera2::set_frame_queue_dst_ops(const camera2_device_t *d,
        const camera2_frame_queue_dst_ops *queue_dst_ops) {
    EmulatedCamera2* ec = getInstance(d);
    ec->mFrameQueueDst = queue_dst_ops;
    return NO_ERROR;
}

int EmulatedCamera2::get_in_progress_count(const camera2_device_t *d) {
    EmulatedCamera2* ec = getInstance(d);
    return ec->getInProgressCount();
}

int EmulatedCamera2::flush_captures_in_progress(const camera2_device_t *d) {
    EmulatedCamera2* ec = getInstance(d);
    return ec->flushCapturesInProgress();
}

int EmulatedCamera2::construct_default_request(const camera2_device_t *d,
        int request_template,
        camera_metadata_t **request) {
    EmulatedCamera2* ec = getInstance(d);
    return ec->constructDefaultRequest(request_template, request);
}

int EmulatedCamera2::allocate_stream(const camera2_device_t *d,
        uint32_t width,
        uint32_t height,
        int format,
        const camera2_stream_ops_t *stream_ops,
        uint32_t *stream_id,
        uint32_t *format_actual,
        uint32_t *usage,
        uint32_t *max_buffers) {
    EmulatedCamera2* ec = getInstance(d);
    return ec->allocateStream(width, height, format, stream_ops,
            stream_id, format_actual, usage, max_buffers);
}

int EmulatedCamera2::register_stream_buffers(const camera2_device_t *d,
        uint32_t stream_id,
        int num_buffers,
        buffer_handle_t *buffers) {
    EmulatedCamera2* ec = getInstance(d);
    return ec->registerStreamBuffers(stream_id,
            num_buffers,
            buffers);
}
int EmulatedCamera2::release_stream(const camera2_device_t *d,
        uint32_t stream_id) {
    EmulatedCamera2* ec = getInstance(d);
    return ec->releaseStream(stream_id);
}

int EmulatedCamera2::allocate_reprocess_stream(const camera2_device_t *d,
        uint32_t width,
        uint32_t height,
        uint32_t format,
        const camera2_stream_in_ops_t *reprocess_stream_ops,
        uint32_t *stream_id,
        uint32_t *consumer_usage,
        uint32_t *max_buffers) {
    EmulatedCamera2* ec = getInstance(d);
    return ec->allocateReprocessStream(width, height, format,
            reprocess_stream_ops, stream_id, consumer_usage, max_buffers);
}

int EmulatedCamera2::allocate_reprocess_stream_from_stream(
            const camera2_device_t *d,
            uint32_t output_stream_id,
            const camera2_stream_in_ops_t *reprocess_stream_ops,
            uint32_t *stream_id) {
    EmulatedCamera2* ec = getInstance(d);
    return ec->allocateReprocessStreamFromStream(output_stream_id,
            reprocess_stream_ops, stream_id);
}


int EmulatedCamera2::release_reprocess_stream(const camera2_device_t *d,
        uint32_t stream_id) {
    EmulatedCamera2* ec = getInstance(d);
    return ec->releaseReprocessStream(stream_id);
}

int EmulatedCamera2::trigger_action(const camera2_device_t *d,
        uint32_t trigger_id,
        int ext1,
        int ext2) {
    EmulatedCamera2* ec = getInstance(d);
    return ec->triggerAction(trigger_id, ext1, ext2);
}

int EmulatedCamera2::set_notify_callback(const camera2_device_t *d,
        camera2_notify_callback notify_cb, void* user) {
    EmulatedCamera2* ec = getInstance(d);
    Mutex::Autolock l(ec->mMutex);
    ec->mNotifyCb = notify_cb;
    ec->mNotifyUserPtr = user;
    return NO_ERROR;
}

int EmulatedCamera2::get_metadata_vendor_tag_ops(const camera2_device_t *d,
        vendor_tag_query_ops_t **ops) {
    EmulatedCamera2* ec = getInstance(d);
    *ops = static_cast<vendor_tag_query_ops_t*>(
            &ec->mVendorTagOps);
    return NO_ERROR;
}

const char* EmulatedCamera2::get_camera_vendor_section_name(
        const vendor_tag_query_ops_t *v,
        uint32_t tag) {
    EmulatedCamera2* ec = static_cast<const TagOps*>(v)->parent;
    return ec->getVendorSectionName(tag);
}

const char* EmulatedCamera2::get_camera_vendor_tag_name(
        const vendor_tag_query_ops_t *v,
        uint32_t tag) {
    EmulatedCamera2* ec = static_cast<const TagOps*>(v)->parent;
    return ec->getVendorTagName(tag);
}

int EmulatedCamera2::get_camera_vendor_tag_type(
        const vendor_tag_query_ops_t *v,
        uint32_t tag)  {
    EmulatedCamera2* ec = static_cast<const TagOps*>(v)->parent;
    return ec->getVendorTagType(tag);
}

int EmulatedCamera2::dump(const camera2_device_t *d, int fd) {
    EmulatedCamera2* ec = getInstance(d);
    return ec->dump(fd);
}

int EmulatedCamera2::close(struct hw_device_t* device) {
    EmulatedCamera2* ec =
            static_cast<EmulatedCamera2*>(
                reinterpret_cast<camera2_device_t*>(device) );
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera2 device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->closeCamera();
}

void EmulatedCamera2::sendNotification(int32_t msgType,
        int32_t ext1, int32_t ext2, int32_t ext3) {
    camera2_notify_callback notifyCb;
    {
        Mutex::Autolock l(mMutex);
        notifyCb = mNotifyCb;
    }
    if (notifyCb != NULL) {
        notifyCb(msgType, ext1, ext2, ext3, mNotifyUserPtr);
    }
}

camera2_device_ops_t EmulatedCamera2::sDeviceOps = {
    EmulatedCamera2::set_request_queue_src_ops,
    EmulatedCamera2::notify_request_queue_not_empty,
    EmulatedCamera2::set_frame_queue_dst_ops,
    EmulatedCamera2::get_in_progress_count,
    EmulatedCamera2::flush_captures_in_progress,
    EmulatedCamera2::construct_default_request,
    EmulatedCamera2::allocate_stream,
    EmulatedCamera2::register_stream_buffers,
    EmulatedCamera2::release_stream,
    EmulatedCamera2::allocate_reprocess_stream,
    EmulatedCamera2::allocate_reprocess_stream_from_stream,
    EmulatedCamera2::release_reprocess_stream,
    EmulatedCamera2::trigger_action,
    EmulatedCamera2::set_notify_callback,
    EmulatedCamera2::get_metadata_vendor_tag_ops,
    EmulatedCamera2::dump
};

}; /* namespace android */
