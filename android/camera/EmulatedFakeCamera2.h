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

#ifndef HW_EMULATOR_CAMERA_EMULATED_FAKE_CAMERA2_H
#define HW_EMULATOR_CAMERA_EMULATED_FAKE_CAMERA2_H

/*
 * Contains declaration of a class EmulatedFakeCamera2 that encapsulates
 * functionality of a fake camera that implements version 2 of the camera device
 * interface.
 */

#include "EmulatedCamera2.h"
#include "fake-pipeline2/Base.h"
#include "fake-pipeline2/Sensor.h"
#include "fake-pipeline2/JpegCompressor.h"
#include <utils/Condition.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/String16.h>

namespace android {

/* Encapsulates functionality of an advanced fake camera.  This camera contains
 * a simple simulation of a scene, sensor, and image processing pipeline.
 */
class EmulatedFakeCamera2 : public EmulatedCamera2 {
public:
    /* Constructs EmulatedFakeCamera instance. */
    EmulatedFakeCamera2(int cameraId, bool facingBack, struct hw_module_t* module);

    /* Destructs EmulatedFakeCamera instance. */
    ~EmulatedFakeCamera2();

    /****************************************************************************
     * EmulatedCamera2 virtual overrides.
     ***************************************************************************/

public:
    /* Initializes EmulatedFakeCamera2 instance. */
    status_t Initialize();

    /****************************************************************************
     * Camera Module API and generic hardware device API implementation
     ***************************************************************************/
public:

    virtual status_t connectCamera(hw_device_t** device);

    virtual status_t plugCamera();
    virtual status_t unplugCamera();
    virtual camera_device_status_t getHotplugStatus();

    virtual status_t closeCamera();

    virtual status_t getCameraInfo(struct camera_info *info);

    /****************************************************************************
     * EmulatedCamera2 abstract API implementation.
     ***************************************************************************/
protected:
    /** Request input queue */

    virtual int requestQueueNotify();

    /** Count of requests in flight */
    virtual int getInProgressCount();

    /** Cancel all captures in flight */
    //virtual int flushCapturesInProgress();

    /** Construct default request */
    virtual int constructDefaultRequest(
            int request_template,
            camera_metadata_t **request);

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

    // virtual int allocateReprocessStream(
    //         uint32_t width,
    //         uint32_t height,
    //         uint32_t format,
    //         const camera2_stream_ops_t *stream_ops,
    //         uint32_t *stream_id,
    //         uint32_t *format_actual,
    //         uint32_t *usage,
    //         uint32_t *max_buffers);

    virtual int allocateReprocessStreamFromStream(
            uint32_t output_stream_id,
            const camera2_stream_in_ops_t *stream_ops,
            uint32_t *stream_id);

    virtual int releaseReprocessStream(uint32_t stream_id);

    virtual int triggerAction(uint32_t trigger_id,
            int32_t ext1,
            int32_t ext2);

    /** Debug methods */

    virtual int dump(int fd);

public:
    /****************************************************************************
     * Utility methods called by configure/readout threads and pipeline
     ***************************************************************************/

    // Get information about a given stream. Will lock mMutex
    const Stream &getStreamInfo(uint32_t streamId);
    const ReprocessStream &getReprocessStreamInfo(uint32_t streamId);

    // Notifies rest of camera subsystem of serious error
    void signalError();

private:
    /****************************************************************************
     * Utility methods
     ***************************************************************************/
    /** Construct static camera metadata, two-pass */
    status_t constructStaticInfo(
            camera_metadata_t **info,
            bool sizeRequest) const;

    /** Two-pass implementation of constructDefaultRequest */
    status_t constructDefaultRequest(
            int request_template,
            camera_metadata_t **request,
            bool sizeRequest) const;
    /** Helper function for constructDefaultRequest */
    static status_t addOrSize( camera_metadata_t *request,
            bool sizeRequest,
            size_t *entryCount,
            size_t *dataCount,
            uint32_t tag,
            const void *entry_data,
            size_t entry_count);

    /** Determine if the stream id is listed in any currently-in-flight
     * requests. Assumes mMutex is locked */
    bool isStreamInUse(uint32_t streamId);

    /** Determine if the reprocess stream id is listed in any
     * currently-in-flight requests. Assumes mMutex is locked */
    bool isReprocessStreamInUse(uint32_t streamId);

    /****************************************************************************
     * Pipeline controller threads
     ***************************************************************************/

    class ConfigureThread: public Thread {
      public:
        ConfigureThread(EmulatedFakeCamera2 *parent);
        ~ConfigureThread();

        status_t waitUntilRunning();
        status_t newRequestAvailable();
        status_t readyToRun();

        bool isStreamInUse(uint32_t id);
        int getInProgressCount();
      private:
        EmulatedFakeCamera2 *mParent;
        static const nsecs_t kWaitPerLoop = 10000000L; // 10 ms

        bool mRunning;
        bool threadLoop();

        bool setupCapture();
        bool setupReprocess();

        bool configureNextCapture();
        bool configureNextReprocess();

        bool getBuffers();

        Mutex mInputMutex; // Protects mActive, mRequestCount
        Condition mInputSignal;
        bool mActive; // Whether we're waiting for input requests or actively
                      // working on them
        size_t mRequestCount;

        camera_metadata_t *mRequest;

        Mutex mInternalsMutex; // Lock before accessing below members.
        bool    mWaitingForReadout;
        bool    mNextNeedsJpeg;
        bool    mNextIsCapture;
        int32_t mNextFrameNumber;
        int64_t mNextExposureTime;
        int64_t mNextFrameDuration;
        int32_t mNextSensitivity;
        Buffers *mNextBuffers;
    };

    class ReadoutThread: public Thread, private JpegCompressor::JpegListener {
      public:
        ReadoutThread(EmulatedFakeCamera2 *parent);
        ~ReadoutThread();

        status_t readyToRun();

        // Input
        status_t waitUntilRunning();
        bool waitForReady(nsecs_t timeout);
        void setNextOperation(bool isCapture,
                camera_metadata_t *request,
                Buffers *buffers);
        bool isStreamInUse(uint32_t id);
        int getInProgressCount();
      private:
        EmulatedFakeCamera2 *mParent;

        bool mRunning;
        bool threadLoop();

        bool readyForNextCapture();
        status_t collectStatisticsMetadata(camera_metadata_t *frame);

        // Inputs
        Mutex mInputMutex; // Protects mActive, mInFlightQueue, mRequestCount
        Condition mInputSignal;
        Condition mReadySignal;

        bool mActive;

        static const int kInFlightQueueSize = 4;
        struct InFlightQueue {
            bool isCapture;
            camera_metadata_t *request;
            Buffers *buffers;
        } *mInFlightQueue;

        size_t mInFlightHead;
        size_t mInFlightTail;

        size_t mRequestCount;

        // Internals
        Mutex mInternalsMutex;

        bool mIsCapture;
        camera_metadata_t *mRequest;
        Buffers *mBuffers;

        // Jpeg completion listeners
        void onJpegDone(const StreamBuffer &jpegBuffer, bool success);
        void onJpegInputDone(const StreamBuffer &inputBuffer);
        nsecs_t mJpegTimestamp;
    };

    // 3A management thread (auto-exposure, focus, white balance)
    class ControlThread: public Thread {
      public:
        ControlThread(EmulatedFakeCamera2 *parent);
        ~ControlThread();

        status_t readyToRun();

        status_t waitUntilRunning();

        // Interpret request's control parameters and override
        // capture settings as needed
        status_t processRequest(camera_metadata_t *request);

        status_t triggerAction(uint32_t msgType,
                int32_t ext1, int32_t ext2);
      private:
        ControlThread(const ControlThread &t);
        ControlThread& operator=(const ControlThread &t);

        // Constants controlling fake 3A behavior
        static const nsecs_t kControlCycleDelay;
        static const nsecs_t kMinAfDuration;
        static const nsecs_t kMaxAfDuration;
        static const float kAfSuccessRate;
        static const float kContinuousAfStartRate;

        static const float kAeScanStartRate;
        static const nsecs_t kMinAeDuration;
        static const nsecs_t kMaxAeDuration;
        static const nsecs_t kMinPrecaptureAeDuration;
        static const nsecs_t kMaxPrecaptureAeDuration;

        static const nsecs_t kNormalExposureTime;
        static const nsecs_t kExposureJump;
        static const nsecs_t kMinExposureTime;

        EmulatedFakeCamera2 *mParent;

        bool mRunning;
        bool threadLoop();

        Mutex mInputMutex; // Protects input methods
        Condition mInputSignal;

        // Trigger notifications
        bool mStartAf;
        bool mCancelAf;
        bool mStartPrecapture;

        // Latest state for 3A request fields
        uint8_t mControlMode;

        uint8_t mEffectMode;
        uint8_t mSceneMode;

        uint8_t mAfMode;
        bool mAfModeChange;

        uint8_t mAwbMode;
        uint8_t mAeMode;

        // Latest trigger IDs
        int32_t mAfTriggerId;
        int32_t mPrecaptureTriggerId;

        // Current state for 3A algorithms
        uint8_t mAfState;
        uint8_t mAeState;
        uint8_t mAwbState;
        bool    mAeLock;

        // Current control parameters
        nsecs_t mExposureTime;

        // Private to threadLoop and its utility methods

        nsecs_t mAfScanDuration;
        nsecs_t mAeScanDuration;
        bool mLockAfterPassiveScan;

        // Utility methods for AF
        int processAfTrigger(uint8_t afMode, uint8_t afState);
        int maybeStartAfScan(uint8_t afMode, uint8_t afState);
        int updateAfScan(uint8_t afMode, uint8_t afState, nsecs_t *maxSleep);
        void updateAfState(uint8_t newState, int32_t triggerId);

        // Utility methods for precapture trigger
        int processPrecaptureTrigger(uint8_t aeMode, uint8_t aeState);
        int maybeStartAeScan(uint8_t aeMode, bool aeLock, uint8_t aeState);
        int updateAeScan(uint8_t aeMode, bool aeLock, uint8_t aeState,
                nsecs_t *maxSleep);
        void updateAeState(uint8_t newState, int32_t triggerId);
    };

    /****************************************************************************
     * Static configuration information
     ***************************************************************************/
private:
    static const uint32_t kMaxRawStreamCount = 1;
    static const uint32_t kMaxProcessedStreamCount = 3;
    static const uint32_t kMaxJpegStreamCount = 1;
    static const uint32_t kMaxReprocessStreamCount = 2;
    static const uint32_t kMaxBufferCount = 4;
    static const uint32_t kAvailableFormats[];
    static const uint32_t kAvailableRawSizes[];
    static const uint64_t kAvailableRawMinDurations[];
    static const uint32_t kAvailableProcessedSizesBack[];
    static const uint32_t kAvailableProcessedSizesFront[];
    static const uint64_t kAvailableProcessedMinDurations[];
    static const uint32_t kAvailableJpegSizesBack[];
    static const uint32_t kAvailableJpegSizesFront[];
    static const uint64_t kAvailableJpegMinDurations[];

    /****************************************************************************
     * Data members.
     ***************************************************************************/

protected:
    /* Facing back (true) or front (false) switch. */
    bool mFacingBack;

private:
    bool mIsConnected;

    /** Stream manipulation */
    uint32_t mNextStreamId;
    uint32_t mRawStreamCount;
    uint32_t mProcessedStreamCount;
    uint32_t mJpegStreamCount;

    uint32_t mNextReprocessStreamId;
    uint32_t mReprocessStreamCount;

    KeyedVector<uint32_t, Stream> mStreams;
    KeyedVector<uint32_t, ReprocessStream> mReprocessStreams;

    /** Simulated hardware interfaces */
    sp<Sensor> mSensor;
    sp<JpegCompressor> mJpegCompressor;

    /** Pipeline control threads */
    sp<ConfigureThread> mConfigureThread;
    sp<ReadoutThread>   mReadoutThread;
    sp<ControlThread>   mControlThread;
};

}; /* namespace android */

#endif  /* HW_EMULATOR_CAMERA_EMULATED_FAKE_CAMERA2_H */
