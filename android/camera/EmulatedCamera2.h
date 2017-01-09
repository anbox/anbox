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

#ifndef HW_EMULATOR_CAMERA_EMULATED_CAMERA2_H
#define HW_EMULATOR_CAMERA_EMULATED_CAMERA2_H

/*
 * Contains declaration of a class EmulatedCamera that encapsulates
 * functionality common to all version 2.0 emulated camera devices.  Instances
 * of this class (for each emulated camera) are created during the construction
 * of the EmulatedCameraFactory instance.  This class serves as an entry point
 * for all camera API calls that defined by camera2_device_ops_t API.
 */

#include "hardware/camera2.h"
#include "system/camera_metadata.h"
#include "EmulatedBaseCamera.h"
#include <utils/Thread.h>
#include <utils/Mutex.h>

namespace android {

/* Encapsulates functionality common to all version 2.0 emulated camera devices
 *
 * Note that EmulatedCameraFactory instantiates object of this class just once,
 * when EmulatedCameraFactory instance gets constructed. Connection to /
 * disconnection from the actual camera device is handled by calls to
 * connectDevice(), and closeCamera() methods of this class that are invoked in
 * response to hw_module_methods_t::open, and camera_device::close callbacks.
 */
class EmulatedCamera2 : public camera2_device, public EmulatedBaseCamera {
public:
    /* Constructs EmulatedCamera2 instance.
     * Param:
     *  cameraId - Zero based camera identifier, which is an index of the camera
     *      instance in camera factory's array.
     *  module - Emulated camera HAL module descriptor.
     */
    EmulatedCamera2(int cameraId,
            struct hw_module_t* module);

    /* Destructs EmulatedCamera2 instance. */
    virtual ~EmulatedCamera2();

    /****************************************************************************
     * Abstract API
     ***************************************************************************/

public:

    /****************************************************************************
     * Public API
     ***************************************************************************/

public:
    virtual status_t Initialize();

    /****************************************************************************
     * Camera module API and generic hardware device API implementation
     ***************************************************************************/

public:
    virtual status_t connectCamera(hw_device_t** device);

    virtual status_t closeCamera();

    virtual status_t getCameraInfo(struct camera_info* info) = 0;

    /****************************************************************************
     * Camera API implementation.
     * These methods are called from the camera API callback routines.
     ***************************************************************************/

protected:
    /** Request input queue notification */
    virtual int requestQueueNotify();

    /** Count of requests in flight */
    virtual int getInProgressCount();

    /** Cancel all captures in flight */
    virtual int flushCapturesInProgress();

    virtual int constructDefaultRequest(
        int request_template,
        camera_metadata_t **request);

    /** Output stream creation and management */
    virtual int allocateStream(
            uint32_t width,
            uint32_t height,
            int format,
            const camera2_stream_ops_t *stream_ops,
            uint32_t *stream_id,
            uint32_t *format_actual,
            uint32_t *usage,
            uint32_t *max_buffers);

    virtual int registerStreamBuffers(
            uint32_t stream_id,
            int num_buffers,
            buffer_handle_t *buffers);

    virtual int releaseStream(uint32_t stream_id);

    /** Input stream creation and management */
    virtual int allocateReprocessStream(
            uint32_t width,
            uint32_t height,
            uint32_t format,
            const camera2_stream_in_ops_t *reprocess_stream_ops,
            uint32_t *stream_id,
            uint32_t *consumer_usage,
            uint32_t *max_buffers);

    virtual int allocateReprocessStreamFromStream(
            uint32_t output_stream_id,
            const camera2_stream_in_ops_t *reprocess_stream_ops,
            uint32_t *stream_id);

    virtual int releaseReprocessStream(uint32_t stream_id);

    /** 3A action triggering */
    virtual int triggerAction(uint32_t trigger_id,
            int32_t ext1, int32_t ext2);

    /** Custom tag definitions */
    virtual const char* getVendorSectionName(uint32_t tag);
    virtual const char* getVendorTagName(uint32_t tag);
    virtual int         getVendorTagType(uint32_t tag);

    /** Debug methods */

    virtual int dump(int fd);

    /****************************************************************************
     * Camera API callbacks as defined by camera2_device_ops structure.  See
     * hardware/libhardware/include/hardware/camera2.h for information on each
     * of these callbacks. Implemented in this class, these callbacks simply
     * dispatch the call into an instance of EmulatedCamera2 class defined in
     * the 'camera_device2' parameter.
     ***************************************************************************/

private:
    /** Input request queue */
    static int set_request_queue_src_ops(const camera2_device_t *,
            const camera2_request_queue_src_ops *queue_src_ops);
    static int notify_request_queue_not_empty(const camera2_device_t *);

    /** Output frame queue */
    static int set_frame_queue_dst_ops(const camera2_device_t *,
            const camera2_frame_queue_dst_ops *queue_dst_ops);

    /** In-progress request management */
    static int get_in_progress_count(const camera2_device_t *);

    static int flush_captures_in_progress(const camera2_device_t *);

    /** Request template creation */
    static int construct_default_request(const camera2_device_t *,
            int request_template,
            camera_metadata_t **request);

    /** Stream management */
    static int allocate_stream(const camera2_device_t *,
            uint32_t width,
            uint32_t height,
            int format,
            const camera2_stream_ops_t *stream_ops,
            uint32_t *stream_id,
            uint32_t *format_actual,
            uint32_t *usage,
            uint32_t *max_buffers);

    static int register_stream_buffers(const camera2_device_t *,
            uint32_t stream_id,
            int num_buffers,
            buffer_handle_t *buffers);

    static int release_stream(const camera2_device_t *,
            uint32_t stream_id);

    static int allocate_reprocess_stream(const camera2_device_t *,
            uint32_t width,
            uint32_t height,
            uint32_t format,
            const camera2_stream_in_ops_t *reprocess_stream_ops,
            uint32_t *stream_id,
            uint32_t *consumer_usage,
            uint32_t *max_buffers);

    static int allocate_reprocess_stream_from_stream(const camera2_device_t *,
            uint32_t output_stream_id,
            const camera2_stream_in_ops_t *reprocess_stream_ops,
            uint32_t *stream_id);

    static int release_reprocess_stream(const camera2_device_t *,
            uint32_t stream_id);

    /** 3A triggers*/
    static int trigger_action(const camera2_device_t *,
            uint32_t trigger_id,
            int ext1,
            int ext2);

    /** Notifications to application */
    static int set_notify_callback(const camera2_device_t *,
            camera2_notify_callback notify_cb,
            void *user);

    /** Vendor metadata registration */
    static int get_metadata_vendor_tag_ops(const camera2_device_t *,
            vendor_tag_query_ops_t **ops);
    // for get_metadata_vendor_tag_ops
    static const char* get_camera_vendor_section_name(
            const vendor_tag_query_ops_t *,
            uint32_t tag);
    static const char* get_camera_vendor_tag_name(
            const vendor_tag_query_ops_t *,
            uint32_t tag);
    static int get_camera_vendor_tag_type(
            const vendor_tag_query_ops_t *,
            uint32_t tag);

    static int dump(const camera2_device_t *, int fd);

    /** For hw_device_t ops */
    static int close(struct hw_device_t* device);

    /****************************************************************************
     * Data members shared with implementations
     ***************************************************************************/
  protected:
    /** Mutex for calls through camera2 device interface */
    Mutex mMutex;

    bool mStatusPresent;

    const camera2_request_queue_src_ops *mRequestQueueSrc;
    const camera2_frame_queue_dst_ops *mFrameQueueDst;

    struct TagOps : public vendor_tag_query_ops {
        EmulatedCamera2 *parent;
    };
    TagOps      mVendorTagOps;

    void sendNotification(int32_t msgType,
            int32_t ext1, int32_t ext2, int32_t ext3);

    /****************************************************************************
     * Data members
     ***************************************************************************/
  private:
    static camera2_device_ops_t sDeviceOps;
    camera2_notify_callback mNotifyCb;
    void* mNotifyUserPtr;
};

}; /* namespace android */

#endif  /* HW_EMULATOR_CAMERA_EMULATED_CAMERA2_H */
