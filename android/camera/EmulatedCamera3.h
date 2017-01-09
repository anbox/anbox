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

#ifndef HW_EMULATOR_CAMERA_EMULATED_CAMERA3_H
#define HW_EMULATOR_CAMERA_EMULATED_CAMERA3_H

/**
 * Contains declaration of a class EmulatedCamera that encapsulates
 * functionality common to all version 3.0 emulated camera devices.  Instances
 * of this class (for each emulated camera) are created during the construction
 * of the EmulatedCameraFactory instance.  This class serves as an entry point
 * for all camera API calls that defined by camera3_device_ops_t API.
 */

#include "hardware/camera3.h"
#include "system/camera_metadata.h"
#include "EmulatedBaseCamera.h"

namespace android {

/**
 * Encapsulates functionality common to all version 3.0 emulated camera devices
 *
 * Note that EmulatedCameraFactory instantiates an object of this class just
 * once, when EmulatedCameraFactory instance gets constructed. Connection to /
 * disconnection from the actual camera device is handled by calls to
 * connectDevice(), and closeCamera() methods of this class that are invoked in
 * response to hw_module_methods_t::open, and camera_device::close callbacks.
 */
class EmulatedCamera3 : public camera3_device, public EmulatedBaseCamera {
public:
    /* Constructs EmulatedCamera3 instance.
     * Param:
     *  cameraId - Zero based camera identifier, which is an index of the camera
     *      instance in camera factory's array.
     *  module - Emulated camera HAL module descriptor.
     */
    EmulatedCamera3(int cameraId,
            struct hw_module_t* module);

    /* Destructs EmulatedCamera2 instance. */
    virtual ~EmulatedCamera3();

    /* List of all defined capabilities plus useful HW levels */
    enum AvailableCapabilities {
        BACKWARD_COMPATIBLE,
        MANUAL_SENSOR,
        MANUAL_POST_PROCESSING,
        RAW,
        PRIVATE_REPROCESSING,
        READ_SENSOR_SETTINGS,
        BURST_CAPTURE,
        YUV_REPROCESSING,
        DEPTH_OUTPUT,
        CONSTRAINED_HIGH_SPEED_VIDEO,
        // Levels
        FULL_LEVEL,

        NUM_CAPABILITIES
    };

    // Char strings for above enum, with size NUM_CAPABILITIES
    static const char *sAvailableCapabilitiesStrings[];

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

    virtual status_t getCameraInfo(struct camera_info* info);

    /****************************************************************************
     * Camera API implementation.
     * These methods are called from the camera API callback routines.
     ***************************************************************************/

protected:

    virtual status_t initializeDevice(
        const camera3_callback_ops *callbackOps);

    virtual status_t configureStreams(
        camera3_stream_configuration *streamList);

    virtual status_t registerStreamBuffers(
        const camera3_stream_buffer_set *bufferSet) ;

    virtual const camera_metadata_t* constructDefaultRequestSettings(
        int type);

    virtual status_t processCaptureRequest(camera3_capture_request *request);

    virtual status_t flush();

    /** Debug methods */

    virtual void dump(int fd);

    /****************************************************************************
     * Camera API callbacks as defined by camera3_device_ops structure.  See
     * hardware/libhardware/include/hardware/camera3.h for information on each
     * of these callbacks. Implemented in this class, these callbacks simply
     * dispatch the call into an instance of EmulatedCamera3 class defined in
     * the 'camera_device3' parameter.
     ***************************************************************************/

private:

    /** Startup */
    static int initialize(const struct camera3_device *,
            const camera3_callback_ops_t *callback_ops);

    /** Stream configuration and buffer registration */

    static int configure_streams(const struct camera3_device *,
            camera3_stream_configuration_t *stream_list);

    static int register_stream_buffers(const struct camera3_device *,
            const camera3_stream_buffer_set_t *buffer_set);

    /** Template request settings provision */

    static const camera_metadata_t* construct_default_request_settings(
            const struct camera3_device *, int type);

    /** Submission of capture requests to HAL */

    static int process_capture_request(const struct camera3_device *,
            camera3_capture_request_t *request);

    static void dump(const camera3_device_t *, int fd);

    static int flush(const camera3_device_t *);

    /** For hw_device_t ops */
    static int close(struct hw_device_t* device);

    /****************************************************************************
     * Data members shared with implementations
     ***************************************************************************/
  protected:

    enum {
        // State at construction time, and after a device operation error
        STATUS_ERROR = 0,
        // State after startup-time init and after device instance close
        STATUS_CLOSED,
        // State after being opened, before device instance init
        STATUS_OPEN,
        // State after device instance initialization
        STATUS_READY,
        // State while actively capturing data
        STATUS_ACTIVE
    } mStatus;

    /**
     * Callbacks back to the framework
     */

    void sendCaptureResult(camera3_capture_result_t *result);
    void sendNotify(camera3_notify_msg_t *msg);

    /****************************************************************************
     * Data members
     ***************************************************************************/
  private:
    static camera3_device_ops_t   sDeviceOps;
    const camera3_callback_ops_t *mCallbackOps;
};

}; /* namespace android */

#endif  /* HW_EMULATOR_CAMERA_EMULATED_CAMERA3_H */
