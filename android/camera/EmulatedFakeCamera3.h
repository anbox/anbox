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

#ifndef HW_EMULATOR_CAMERA_EMULATED_FAKE_CAMERA3_H
#define HW_EMULATOR_CAMERA_EMULATED_FAKE_CAMERA3_H

/**
 * Contains declaration of a class EmulatedCamera that encapsulates
 * functionality of a fake camera that implements version 3 of the camera device
 * interace.
 */

#include "EmulatedCamera3.h"
#include "fake-pipeline2/Base.h"
#include "fake-pipeline2/Sensor.h"
#include "fake-pipeline2/JpegCompressor.h"
#include <camera/CameraMetadata.h>
#include <utils/SortedVector.h>
#include <utils/List.h>
#include <utils/Mutex.h>

namespace android {

/**
 * Encapsulates functionality for a v3 HAL camera which produces synthetic data.
 *
 * Note that EmulatedCameraFactory instantiates an object of this class just
 * once, when EmulatedCameraFactory instance gets constructed. Connection to /
 * disconnection from the actual camera device is handled by calls to
 * connectDevice(), and closeCamera() methods of this class that are invoked in
 * response to hw_module_methods_t::open, and camera_device::close callbacks.
 */
class EmulatedFakeCamera3 : public EmulatedCamera3,
        private Sensor::SensorListener {
public:

    EmulatedFakeCamera3(int cameraId, bool facingBack,
            struct hw_module_t* module);

    virtual ~EmulatedFakeCamera3();

    /****************************************************************************
     * EmulatedCamera3 virtual overrides
     ***************************************************************************/

public:

    virtual status_t Initialize();

    /****************************************************************************
     * Camera module API and generic hardware device API implementation
     ***************************************************************************/

public:
    virtual status_t connectCamera(hw_device_t** device);

    virtual status_t closeCamera();

    virtual status_t getCameraInfo(struct camera_info *info);

    /****************************************************************************
     * EmulatedCamera3 abstract API implementation
     ***************************************************************************/

protected:

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

private:

    /**
     * Get the requested capability set for this camera
     */
    status_t getCameraCapabilities();

    bool hasCapability(AvailableCapabilities cap);

    /**
     * Build the static info metadata buffer for this device
     */
    status_t constructStaticInfo();

    /**
     * Run the fake 3A algorithms as needed. May override/modify settings
     * values.
     */
    status_t process3A(CameraMetadata &settings);

    status_t doFakeAE(CameraMetadata &settings);
    status_t doFakeAF(CameraMetadata &settings);
    status_t doFakeAWB(CameraMetadata &settings);
    void     update3A(CameraMetadata &settings);

    /** Signal from readout thread that it doesn't have anything to do */
    void     signalReadoutIdle();

    /** Handle interrupt events from the sensor */
    void     onSensorEvent(uint32_t frameNumber, Event e, nsecs_t timestamp);

    /****************************************************************************
     * Static configuration information
     ***************************************************************************/
private:
    static const uint32_t kMaxRawStreamCount = 1;
    static const uint32_t kMaxProcessedStreamCount = 3;
    static const uint32_t kMaxJpegStreamCount = 1;
    static const uint32_t kMaxReprocessStreamCount = 2;
    static const uint32_t kMaxBufferCount = 4;
    // We need a positive stream ID to distinguish external buffers from
    // sensor-generated buffers which use a nonpositive ID. Otherwise, HAL3 has
    // no concept of a stream id.
    static const uint32_t kGenericStreamId = 1;
    static const int32_t  kAvailableFormats[];

    static const int64_t  kSyncWaitTimeout     = 10000000; // 10 ms
    static const int32_t  kMaxSyncTimeoutCount = 1000; // 1000 kSyncWaitTimeouts
    static const uint32_t kFenceTimeoutMs      = 2000; // 2 s

    /****************************************************************************
     * Data members.
     ***************************************************************************/

    /* HAL interface serialization lock. */
    Mutex              mLock;

    /* Facing back (true) or front (false) switch. */
    bool               mFacingBack;

    SortedVector<AvailableCapabilities> mCapabilities;

    /**
     * Cache for default templates. Once one is requested, the pointer must be
     * valid at least until close() is called on the device
     */
    camera_metadata_t *mDefaultTemplates[CAMERA3_TEMPLATE_COUNT];

    /**
     * Private stream information, stored in camera3_stream_t->priv.
     */
    struct PrivateStreamInfo {
        bool alive;
    };

    // Shortcut to the input stream
    camera3_stream_t*  mInputStream;

    typedef List<camera3_stream_t*>           StreamList;
    typedef List<camera3_stream_t*>::iterator StreamIterator;
    typedef Vector<camera3_stream_buffer>     HalBufferVector;

    // All streams, including input stream
    StreamList         mStreams;

    // Cached settings from latest submitted request
    CameraMetadata     mPrevSettings;

    /** Fake hardware interfaces */
    sp<Sensor>         mSensor;
    sp<JpegCompressor> mJpegCompressor;
    friend class       JpegCompressor;

    /** Processing thread for sending out results */

    class ReadoutThread : public Thread, private JpegCompressor::JpegListener {
      public:
        ReadoutThread(EmulatedFakeCamera3 *parent);
        ~ReadoutThread();

        struct Request {
            uint32_t         frameNumber;
            CameraMetadata   settings;
            HalBufferVector *buffers;
            Buffers         *sensorBuffers;
        };

        /**
         * Interface to parent class
         */

        // Place request in the in-flight queue to wait for sensor capture
        void     queueCaptureRequest(const Request &r);

        // Test if the readout thread is idle (no in-flight requests, not
        // currently reading out anything
        bool     isIdle();

        // Wait until isIdle is true
        status_t waitForReadout();

      private:
        static const nsecs_t kWaitPerLoop  = 10000000L; // 10 ms
        static const nsecs_t kMaxWaitLoops = 1000;
        static const size_t  kMaxQueueSize = 2;

        EmulatedFakeCamera3 *mParent;
        Mutex mLock;

        List<Request> mInFlightQueue;
        Condition     mInFlightSignal;
        bool          mThreadActive;

        virtual bool threadLoop();

        // Only accessed by threadLoop

        Request mCurrentRequest;

        // Jpeg completion callbacks

        Mutex                 mJpegLock;
        bool                  mJpegWaiting;
        camera3_stream_buffer mJpegHalBuffer;
        uint32_t              mJpegFrameNumber;
        virtual void onJpegDone(const StreamBuffer &jpegBuffer, bool success);
        virtual void onJpegInputDone(const StreamBuffer &inputBuffer);
    };

    sp<ReadoutThread> mReadoutThread;

    /** Fake 3A constants */

    static const nsecs_t kNormalExposureTime;
    static const nsecs_t kFacePriorityExposureTime;
    static const int     kNormalSensitivity;
    static const int     kFacePrioritySensitivity;
    // Rate of converging AE to new target value, as fraction of difference between
    // current and target value.
    static const float   kExposureTrackRate;
    // Minimum duration for precapture state. May be longer if slow to converge
    // to target exposure
    static const int     kPrecaptureMinFrames;
    // How often to restart AE 'scanning'
    static const int     kStableAeMaxFrames;
    // Maximum stop below 'normal' exposure time that we'll wander to while
    // pretending to converge AE. In powers of 2. (-2 == 1/4 as bright)
    static const float   kExposureWanderMin;
    // Maximum stop above 'normal' exposure time that we'll wander to while
    // pretending to converge AE. In powers of 2. (2 == 4x as bright)
    static const float   kExposureWanderMax;

    /** Fake 3A state */

    uint8_t mControlMode;
    bool    mFacePriority;
    uint8_t mAeState;
    uint8_t mAfState;
    uint8_t mAwbState;
    uint8_t mAeMode;
    uint8_t mAfMode;
    uint8_t mAwbMode;

    int     mAeCounter;
    nsecs_t mAeCurrentExposureTime;
    nsecs_t mAeTargetExposureTime;
    int     mAeCurrentSensitivity;

};

} // namespace android

#endif // HW_EMULATOR_CAMERA_EMULATED_CAMERA3_H
