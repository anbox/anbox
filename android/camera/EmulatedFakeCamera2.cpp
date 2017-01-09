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
 * Contains implementation of a class EmulatedFakeCamera2 that encapsulates
 * functionality of an advanced fake camera.
 */

#include <inttypes.h>

//#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera_FakeCamera2"
#include <utils/Log.h>

#include "EmulatedFakeCamera2.h"
#include "EmulatedCameraFactory.h"
#include <ui/Rect.h>
#include <ui/GraphicBufferMapper.h>
#include "gralloc_cb.h"

#define ERROR_CAMERA_NOT_PRESENT -EPIPE

#define CAMERA2_EXT_TRIGGER_TESTING_DISCONNECT 0xFFFFFFFF

namespace android {

const int64_t USEC = 1000LL;
const int64_t MSEC = USEC * 1000LL;
const int64_t SEC = MSEC * 1000LL;

const uint32_t EmulatedFakeCamera2::kAvailableFormats[4] = {
        HAL_PIXEL_FORMAT_RAW16,
        HAL_PIXEL_FORMAT_BLOB,
        HAL_PIXEL_FORMAT_RGBA_8888,
        //        HAL_PIXEL_FORMAT_YV12,
        HAL_PIXEL_FORMAT_YCrCb_420_SP
};

const uint32_t EmulatedFakeCamera2::kAvailableRawSizes[2] = {
    640, 480
    //    Sensor::kResolution[0], Sensor::kResolution[1]
};

const uint64_t EmulatedFakeCamera2::kAvailableRawMinDurations[1] = {
    static_cast<uint64_t>(Sensor::kFrameDurationRange[0])
};

const uint32_t EmulatedFakeCamera2::kAvailableProcessedSizesBack[4] = {
    640, 480, 320, 240
    //    Sensor::kResolution[0], Sensor::kResolution[1]
};

const uint32_t EmulatedFakeCamera2::kAvailableProcessedSizesFront[4] = {
    320, 240, 160, 120
    //    Sensor::kResolution[0], Sensor::kResolution[1]
};

const uint64_t EmulatedFakeCamera2::kAvailableProcessedMinDurations[1] = {
    static_cast<uint64_t>(Sensor::kFrameDurationRange[0])
};

const uint32_t EmulatedFakeCamera2::kAvailableJpegSizesBack[2] = {
    640, 480
    //    Sensor::kResolution[0], Sensor::kResolution[1]
};

const uint32_t EmulatedFakeCamera2::kAvailableJpegSizesFront[2] = {
    320, 240
    //    Sensor::kResolution[0], Sensor::kResolution[1]
};


const uint64_t EmulatedFakeCamera2::kAvailableJpegMinDurations[1] = {
    static_cast<uint64_t>(Sensor::kFrameDurationRange[0])
};


EmulatedFakeCamera2::EmulatedFakeCamera2(int cameraId,
        bool facingBack,
        struct hw_module_t* module)
        : EmulatedCamera2(cameraId,module),
          mFacingBack(facingBack),
          mIsConnected(false)
{
    ALOGD("Constructing emulated fake camera 2 facing %s",
            facingBack ? "back" : "front");
}

EmulatedFakeCamera2::~EmulatedFakeCamera2() {
    if (mCameraInfo != NULL) {
        free_camera_metadata(mCameraInfo);
    }
}

/****************************************************************************
 * Public API overrides
 ***************************************************************************/

status_t EmulatedFakeCamera2::Initialize() {
    status_t res;

    res = constructStaticInfo(&mCameraInfo, true);
    if (res != OK) {
        ALOGE("%s: Unable to allocate static info: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return res;
    }
    res = constructStaticInfo(&mCameraInfo, false);
    if (res != OK) {
        ALOGE("%s: Unable to fill in static info: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return res;
    }
    if (res != OK) return res;

    mNextStreamId = 1;
    mNextReprocessStreamId = 1;
    mRawStreamCount = 0;
    mProcessedStreamCount = 0;
    mJpegStreamCount = 0;
    mReprocessStreamCount = 0;

    return NO_ERROR;
}

/****************************************************************************
 * Camera module API overrides
 ***************************************************************************/

status_t EmulatedFakeCamera2::connectCamera(hw_device_t** device) {
    status_t res;
    ALOGV("%s", __FUNCTION__);

    {
        Mutex::Autolock l(mMutex);
        if (!mStatusPresent) {
            ALOGE("%s: Camera ID %d is unplugged", __FUNCTION__,
                  mCameraID);
            return -ENODEV;
        }
    }

    mConfigureThread = new ConfigureThread(this);
    mReadoutThread = new ReadoutThread(this);
    mControlThread = new ControlThread(this);
    mSensor = new Sensor();
    mJpegCompressor = new JpegCompressor();

    mNextStreamId = 1;
    mNextReprocessStreamId = 1;

    res = mSensor->startUp();
    if (res != NO_ERROR) return res;

    res = mConfigureThread->run("EmulatedFakeCamera2::configureThread");
    if (res != NO_ERROR) return res;

    res = mReadoutThread->run("EmulatedFakeCamera2::readoutThread");
    if (res != NO_ERROR) return res;

    res = mControlThread->run("EmulatedFakeCamera2::controlThread");
    if (res != NO_ERROR) return res;

    status_t ret = EmulatedCamera2::connectCamera(device);

    if (ret >= 0) {
        mIsConnected = true;
    }

    return ret;
}

status_t EmulatedFakeCamera2::plugCamera() {
    {
        Mutex::Autolock l(mMutex);

        if (!mStatusPresent) {
            ALOGI("%s: Plugged back in", __FUNCTION__);
            mStatusPresent = true;
        }
    }

    return NO_ERROR;
}

status_t EmulatedFakeCamera2::unplugCamera() {
    {
        Mutex::Autolock l(mMutex);

        if (mStatusPresent) {
            ALOGI("%s: Unplugged camera", __FUNCTION__);
            mStatusPresent = false;
        }
    }

    return closeCamera();
}

camera_device_status_t EmulatedFakeCamera2::getHotplugStatus() {
    Mutex::Autolock l(mMutex);
    return mStatusPresent ?
        CAMERA_DEVICE_STATUS_PRESENT :
        CAMERA_DEVICE_STATUS_NOT_PRESENT;
}



status_t EmulatedFakeCamera2::closeCamera() {
    {
        Mutex::Autolock l(mMutex);

        status_t res;
        ALOGV("%s", __FUNCTION__);

        if (!mIsConnected) {
            return NO_ERROR;
        }

        res = mSensor->shutDown();
        if (res != NO_ERROR) {
            ALOGE("%s: Unable to shut down sensor: %d", __FUNCTION__, res);
            return res;
        }

        mConfigureThread->requestExit();
        mReadoutThread->requestExit();
        mControlThread->requestExit();
        mJpegCompressor->cancel();
    }

    // give up the lock since we will now block and the threads
    // can call back into this object
    mConfigureThread->join();
    mReadoutThread->join();
    mControlThread->join();

    ALOGV("%s exit", __FUNCTION__);

    {
        Mutex::Autolock l(mMutex);
        mIsConnected = false;
    }

    return NO_ERROR;
}

status_t EmulatedFakeCamera2::getCameraInfo(struct camera_info *info) {
    info->facing = mFacingBack ? CAMERA_FACING_BACK : CAMERA_FACING_FRONT;
    info->orientation = gEmulatedCameraFactory.getFakeCameraOrientation();
    return EmulatedCamera2::getCameraInfo(info);
}

/****************************************************************************
 * Camera device API overrides
 ***************************************************************************/

/** Request input queue */

int EmulatedFakeCamera2::requestQueueNotify() {
    ALOGV("Request queue notification received");

    ALOG_ASSERT(mRequestQueueSrc != NULL,
            "%s: Request queue src not set, but received queue notification!",
            __FUNCTION__);
    ALOG_ASSERT(mFrameQueueDst != NULL,
            "%s: Request queue src not set, but received queue notification!",
            __FUNCTION__);
    ALOG_ASSERT(mStreams.size() != 0,
            "%s: No streams allocated, but received queue notification!",
            __FUNCTION__);
    return mConfigureThread->newRequestAvailable();
}

int EmulatedFakeCamera2::getInProgressCount() {
    Mutex::Autolock l(mMutex);

    if (!mStatusPresent) {
        ALOGW("%s: Camera was physically disconnected", __FUNCTION__);
        return ERROR_CAMERA_NOT_PRESENT;
    }

    int requestCount = 0;
    requestCount += mConfigureThread->getInProgressCount();
    requestCount += mReadoutThread->getInProgressCount();
    requestCount += mJpegCompressor->isBusy() ? 1 : 0;

    return requestCount;
}

int EmulatedFakeCamera2::constructDefaultRequest(
        int request_template,
        camera_metadata_t **request) {

    if (request == NULL) return BAD_VALUE;
    if (request_template < 0 || request_template >= CAMERA2_TEMPLATE_COUNT) {
        return BAD_VALUE;
    }

    {
        Mutex::Autolock l(mMutex);
        if (!mStatusPresent) {
            ALOGW("%s: Camera was physically disconnected", __FUNCTION__);
            return ERROR_CAMERA_NOT_PRESENT;
        }
    }

    status_t res;
    // Pass 1, calculate size and allocate
    res = constructDefaultRequest(request_template,
            request,
            true);
    if (res != OK) {
        return res;
    }
    // Pass 2, build request
    res = constructDefaultRequest(request_template,
            request,
            false);
    if (res != OK) {
        ALOGE("Unable to populate new request for template %d",
                request_template);
    }

    return res;
}

int EmulatedFakeCamera2::allocateStream(
        uint32_t width,
        uint32_t height,
        int format,
        const camera2_stream_ops_t *stream_ops,
        uint32_t *stream_id,
        uint32_t *format_actual,
        uint32_t *usage,
        uint32_t *max_buffers) {
    Mutex::Autolock l(mMutex);

    if (!mStatusPresent) {
        ALOGW("%s: Camera was physically disconnected", __FUNCTION__);
        return ERROR_CAMERA_NOT_PRESENT;
    }

    // Temporary shim until FORMAT_ZSL is removed
    if (format == CAMERA2_HAL_PIXEL_FORMAT_ZSL) {
        format = HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED;
    }

    if (format != HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED) {
        unsigned int numFormats = sizeof(kAvailableFormats) / sizeof(uint32_t);
        unsigned int formatIdx = 0;
        unsigned int sizeOffsetIdx = 0;
        for (; formatIdx < numFormats; formatIdx++) {
            if (format == (int)kAvailableFormats[formatIdx]) break;
        }
        if (formatIdx == numFormats) {
            ALOGE("%s: Format 0x%x is not supported", __FUNCTION__, format);
            return BAD_VALUE;
        }
    }

    const uint32_t *availableSizes;
    size_t availableSizeCount;
    switch (format) {
        case HAL_PIXEL_FORMAT_RAW16:
            availableSizes = kAvailableRawSizes;
            availableSizeCount = sizeof(kAvailableRawSizes)/sizeof(uint32_t);
            break;
        case HAL_PIXEL_FORMAT_BLOB:
            availableSizes = mFacingBack ?
                    kAvailableJpegSizesBack : kAvailableJpegSizesFront;
            availableSizeCount = mFacingBack ?
                    sizeof(kAvailableJpegSizesBack)/sizeof(uint32_t) :
                    sizeof(kAvailableJpegSizesFront)/sizeof(uint32_t);
            break;
        case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            availableSizes = mFacingBack ?
                    kAvailableProcessedSizesBack : kAvailableProcessedSizesFront;
            availableSizeCount = mFacingBack ?
                    sizeof(kAvailableProcessedSizesBack)/sizeof(uint32_t) :
                    sizeof(kAvailableProcessedSizesFront)/sizeof(uint32_t);
            break;
        default:
            ALOGE("%s: Unknown format 0x%x", __FUNCTION__, format);
            return BAD_VALUE;
    }

    unsigned int resIdx = 0;
    for (; resIdx < availableSizeCount; resIdx++) {
        if (availableSizes[resIdx * 2] == width &&
                availableSizes[resIdx * 2 + 1] == height) break;
    }
    if (resIdx == availableSizeCount) {
        ALOGE("%s: Format 0x%x does not support resolution %d, %d", __FUNCTION__,
                format, width, height);
        return BAD_VALUE;
    }

    switch (format) {
        case HAL_PIXEL_FORMAT_RAW16:
            if (mRawStreamCount >= kMaxRawStreamCount) {
                ALOGE("%s: Cannot allocate another raw stream (%d already allocated)",
                        __FUNCTION__, mRawStreamCount);
                return INVALID_OPERATION;
            }
            mRawStreamCount++;
            break;
        case HAL_PIXEL_FORMAT_BLOB:
            if (mJpegStreamCount >= kMaxJpegStreamCount) {
                ALOGE("%s: Cannot allocate another JPEG stream (%d already allocated)",
                        __FUNCTION__, mJpegStreamCount);
                return INVALID_OPERATION;
            }
            mJpegStreamCount++;
            break;
        default:
            if (mProcessedStreamCount >= kMaxProcessedStreamCount) {
                ALOGE("%s: Cannot allocate another processed stream (%d already allocated)",
                        __FUNCTION__, mProcessedStreamCount);
                return INVALID_OPERATION;
            }
            mProcessedStreamCount++;
    }

    Stream newStream;
    newStream.ops = stream_ops;
    newStream.width = width;
    newStream.height = height;
    newStream.format = format;
    // TODO: Query stride from gralloc
    newStream.stride = width;

    mStreams.add(mNextStreamId, newStream);

    *stream_id = mNextStreamId;
    if (format_actual) *format_actual = format;
    *usage = GRALLOC_USAGE_HW_CAMERA_WRITE;
    *max_buffers = kMaxBufferCount;

    ALOGV("Stream allocated: %d, %d x %d, 0x%x. U: %x, B: %d",
            *stream_id, width, height, format, *usage, *max_buffers);

    mNextStreamId++;
    return NO_ERROR;
}

int EmulatedFakeCamera2::registerStreamBuffers(
            uint32_t stream_id,
            int num_buffers,
            buffer_handle_t *buffers) {
    Mutex::Autolock l(mMutex);

    if (!mStatusPresent) {
        ALOGW("%s: Camera was physically disconnected", __FUNCTION__);
        return ERROR_CAMERA_NOT_PRESENT;
    }

    ALOGV("%s: Stream %d registering %d buffers", __FUNCTION__,
            stream_id, num_buffers);
    // Need to find out what the final concrete pixel format for our stream is
    // Assumes that all buffers have the same format.
    if (num_buffers < 1) {
        ALOGE("%s: Stream %d only has %d buffers!",
                __FUNCTION__, stream_id, num_buffers);
        return BAD_VALUE;
    }
    const cb_handle_t *streamBuffer =
            reinterpret_cast<const cb_handle_t*>(buffers[0]);

    int finalFormat = streamBuffer->format;

    if (finalFormat == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED) {
        ALOGE("%s: Stream %d: Bad final pixel format "
                "HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED; "
                "concrete pixel format required!", __FUNCTION__, stream_id);
        return BAD_VALUE;
    }

    ssize_t streamIndex = mStreams.indexOfKey(stream_id);
    if (streamIndex < 0) {
        ALOGE("%s: Unknown stream id %d!", __FUNCTION__, stream_id);
        return BAD_VALUE;
    }

    Stream &stream = mStreams.editValueAt(streamIndex);

    ALOGV("%s: Stream %d format set to %x, previously %x",
            __FUNCTION__, stream_id, finalFormat, stream.format);

    stream.format = finalFormat;

    return NO_ERROR;
}

int EmulatedFakeCamera2::releaseStream(uint32_t stream_id) {
    Mutex::Autolock l(mMutex);

    ssize_t streamIndex = mStreams.indexOfKey(stream_id);
    if (streamIndex < 0) {
        ALOGE("%s: Unknown stream id %d!", __FUNCTION__, stream_id);
        return BAD_VALUE;
    }

    if (isStreamInUse(stream_id)) {
        ALOGE("%s: Cannot release stream %d; in use!", __FUNCTION__,
                stream_id);
        return BAD_VALUE;
    }

    switch(mStreams.valueAt(streamIndex).format) {
        case HAL_PIXEL_FORMAT_RAW16:
            mRawStreamCount--;
            break;
        case HAL_PIXEL_FORMAT_BLOB:
            mJpegStreamCount--;
            break;
        default:
            mProcessedStreamCount--;
            break;
    }

    mStreams.removeItemsAt(streamIndex);

    return NO_ERROR;
}

int EmulatedFakeCamera2::allocateReprocessStreamFromStream(
        uint32_t output_stream_id,
        const camera2_stream_in_ops_t *stream_ops,
        uint32_t *stream_id) {
    Mutex::Autolock l(mMutex);

    if (!mStatusPresent) {
        ALOGW("%s: Camera was physically disconnected", __FUNCTION__);
        return ERROR_CAMERA_NOT_PRESENT;
    }

    ssize_t baseStreamIndex = mStreams.indexOfKey(output_stream_id);
    if (baseStreamIndex < 0) {
        ALOGE("%s: Unknown output stream id %d!", __FUNCTION__, output_stream_id);
        return BAD_VALUE;
    }

    const Stream &baseStream = mStreams[baseStreamIndex];

    // We'll reprocess anything we produced

    if (mReprocessStreamCount >= kMaxReprocessStreamCount) {
        ALOGE("%s: Cannot allocate another reprocess stream (%d already allocated)",
                __FUNCTION__, mReprocessStreamCount);
        return INVALID_OPERATION;
    }
    mReprocessStreamCount++;

    ReprocessStream newStream;
    newStream.ops = stream_ops;
    newStream.width = baseStream.width;
    newStream.height = baseStream.height;
    newStream.format = baseStream.format;
    newStream.stride = baseStream.stride;
    newStream.sourceStreamId = output_stream_id;

    *stream_id = mNextReprocessStreamId;
    mReprocessStreams.add(mNextReprocessStreamId, newStream);

    ALOGV("Reprocess stream allocated: %d: %d, %d, 0x%x. Parent stream: %d",
            *stream_id, newStream.width, newStream.height, newStream.format,
            output_stream_id);

    mNextReprocessStreamId++;
    return NO_ERROR;
}

int EmulatedFakeCamera2::releaseReprocessStream(uint32_t stream_id) {
    Mutex::Autolock l(mMutex);

    ssize_t streamIndex = mReprocessStreams.indexOfKey(stream_id);
    if (streamIndex < 0) {
        ALOGE("%s: Unknown reprocess stream id %d!", __FUNCTION__, stream_id);
        return BAD_VALUE;
    }

    if (isReprocessStreamInUse(stream_id)) {
        ALOGE("%s: Cannot release reprocessing stream %d; in use!", __FUNCTION__,
                stream_id);
        return BAD_VALUE;
    }

    mReprocessStreamCount--;
    mReprocessStreams.removeItemsAt(streamIndex);

    return NO_ERROR;
}

int EmulatedFakeCamera2::triggerAction(uint32_t trigger_id,
        int32_t ext1,
        int32_t ext2) {
    Mutex::Autolock l(mMutex);

    if (trigger_id == CAMERA2_EXT_TRIGGER_TESTING_DISCONNECT) {
        ALOGI("%s: Disconnect trigger - camera must be closed", __FUNCTION__);
        mStatusPresent = false;

        gEmulatedCameraFactory.onStatusChanged(
                mCameraID,
                CAMERA_DEVICE_STATUS_NOT_PRESENT);
    }

    if (!mStatusPresent) {
        ALOGW("%s: Camera was physically disconnected", __FUNCTION__);
        return ERROR_CAMERA_NOT_PRESENT;
    }

    return mControlThread->triggerAction(trigger_id,
            ext1, ext2);
}

/** Shutdown and debug methods */

int EmulatedFakeCamera2::dump(int fd) {
    String8 result;

    result.appendFormat("    Camera HAL device: EmulatedFakeCamera2\n");
    result.appendFormat("      Streams:\n");
    for (size_t i = 0; i < mStreams.size(); i++) {
        int id = mStreams.keyAt(i);
        const Stream& s = mStreams.valueAt(i);
        result.appendFormat(
            "         Stream %d: %d x %d, format 0x%x, stride %d\n",
            id, s.width, s.height, s.format, s.stride);
    }

    write(fd, result.string(), result.size());

    return NO_ERROR;
}

void EmulatedFakeCamera2::signalError() {
    // TODO: Let parent know so we can shut down cleanly
    ALOGE("Worker thread is signaling a serious error");
}

/** Pipeline control worker thread methods */

EmulatedFakeCamera2::ConfigureThread::ConfigureThread(EmulatedFakeCamera2 *parent):
        Thread(false),
        mParent(parent),
        mRequestCount(0),
        mNextBuffers(NULL) {
    mRunning = false;
}

EmulatedFakeCamera2::ConfigureThread::~ConfigureThread() {
}

status_t EmulatedFakeCamera2::ConfigureThread::readyToRun() {
    Mutex::Autolock lock(mInputMutex);

    ALOGV("Starting up ConfigureThread");
    mRequest = NULL;
    mActive  = false;
    mRunning = true;

    mInputSignal.signal();
    return NO_ERROR;
}

status_t EmulatedFakeCamera2::ConfigureThread::waitUntilRunning() {
    Mutex::Autolock lock(mInputMutex);
    if (!mRunning) {
        ALOGV("Waiting for configure thread to start");
        mInputSignal.wait(mInputMutex);
    }
    return OK;
}

status_t EmulatedFakeCamera2::ConfigureThread::newRequestAvailable() {
    waitUntilRunning();

    Mutex::Autolock lock(mInputMutex);

    mActive = true;
    mInputSignal.signal();

    return OK;
}

bool EmulatedFakeCamera2::ConfigureThread::isStreamInUse(uint32_t id) {
    Mutex::Autolock lock(mInternalsMutex);

    if (mNextBuffers == NULL) return false;
    for (size_t i=0; i < mNextBuffers->size(); i++) {
        if ((*mNextBuffers)[i].streamId == (int)id) return true;
    }
    return false;
}

int EmulatedFakeCamera2::ConfigureThread::getInProgressCount() {
    Mutex::Autolock lock(mInputMutex);
    return mRequestCount;
}

bool EmulatedFakeCamera2::ConfigureThread::threadLoop() {
    status_t res;

    // Check if we're currently processing or just waiting
    {
        Mutex::Autolock lock(mInputMutex);
        if (!mActive) {
            // Inactive, keep waiting until we've been signaled
            status_t res;
            res = mInputSignal.waitRelative(mInputMutex, kWaitPerLoop);
            if (res != NO_ERROR && res != TIMED_OUT) {
                ALOGE("%s: Error waiting for input requests: %d",
                        __FUNCTION__, res);
                return false;
            }
            if (!mActive) return true;
            ALOGV("New request available");
        }
        // Active
    }

    if (mRequest == NULL) {
        Mutex::Autolock il(mInternalsMutex);

        ALOGV("Configure: Getting next request");
        res = mParent->mRequestQueueSrc->dequeue_request(
            mParent->mRequestQueueSrc,
            &mRequest);
        if (res != NO_ERROR) {
            ALOGE("%s: Error dequeuing next request: %d", __FUNCTION__, res);
            mParent->signalError();
            return false;
        }
        if (mRequest == NULL) {
            ALOGV("Configure: Request queue empty, going inactive");
            // No requests available, go into inactive mode
            Mutex::Autolock lock(mInputMutex);
            mActive = false;
            return true;
        } else {
            Mutex::Autolock lock(mInputMutex);
            mRequestCount++;
        }

        camera_metadata_entry_t type;
        res = find_camera_metadata_entry(mRequest,
                ANDROID_REQUEST_TYPE,
                &type);
        if (res != NO_ERROR) {
            ALOGE("%s: error reading request type", __FUNCTION__);
            mParent->signalError();
            return false;
        }
        bool success = false;;
        switch (type.data.u8[0]) {
            case ANDROID_REQUEST_TYPE_CAPTURE:
                success = setupCapture();
                break;
            case ANDROID_REQUEST_TYPE_REPROCESS:
                success = setupReprocess();
                break;
            default:
                ALOGE("%s: Unexpected request type %d",
                        __FUNCTION__, type.data.u8[0]);
                mParent->signalError();
                break;
        }
        if (!success) return false;

    }

    if (mWaitingForReadout) {
        bool readoutDone;
        readoutDone = mParent->mReadoutThread->waitForReady(kWaitPerLoop);
        if (!readoutDone) return true;

        if (mNextNeedsJpeg) {
            ALOGV("Configure: Waiting for JPEG compressor");
        } else {
            ALOGV("Configure: Waiting for sensor");
        }
        mWaitingForReadout = false;
    }

    if (mNextNeedsJpeg) {
        bool jpegDone;
        jpegDone = mParent->mJpegCompressor->waitForDone(kWaitPerLoop);
        if (!jpegDone) return true;

        ALOGV("Configure: Waiting for sensor");
        mNextNeedsJpeg = false;
    }

    if (mNextIsCapture) {
        return configureNextCapture();
    } else {
        return configureNextReprocess();
    }
}

bool EmulatedFakeCamera2::ConfigureThread::setupCapture() {
    status_t res;

    mNextIsCapture = true;
    // Get necessary parameters for sensor config
    mParent->mControlThread->processRequest(mRequest);

    camera_metadata_entry_t streams;
    res = find_camera_metadata_entry(mRequest,
            ANDROID_REQUEST_OUTPUT_STREAMS,
            &streams);
    if (res != NO_ERROR) {
        ALOGE("%s: error reading output stream tag", __FUNCTION__);
        mParent->signalError();
        return false;
    }

    mNextBuffers = new Buffers;
    mNextNeedsJpeg = false;
    ALOGV("Configure: Setting up buffers for capture");
    for (size_t i = 0; i < streams.count; i++) {
        int streamId = streams.data.i32[i];
        const Stream &s = mParent->getStreamInfo(streamId);
        if (s.format == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED) {
            ALOGE("%s: Stream %d does not have a concrete pixel format, but "
                    "is included in a request!", __FUNCTION__, streamId);
            mParent->signalError();
            return false;
        }
        StreamBuffer b;
        b.streamId = streams.data.u8[i];
        b.width  = s.width;
        b.height = s.height;
        b.format = s.format;
        b.stride = s.stride;
        mNextBuffers->push_back(b);
        ALOGV("Configure:    Buffer %zu: Stream %d, %d x %d, format 0x%x, "
                "stride %d",
                i, b.streamId, b.width, b.height, b.format, b.stride);
        if (b.format == HAL_PIXEL_FORMAT_BLOB) {
            mNextNeedsJpeg = true;
        }
    }

    camera_metadata_entry_t e;
    res = find_camera_metadata_entry(mRequest,
            ANDROID_REQUEST_FRAME_COUNT,
            &e);
    if (res != NO_ERROR) {
        ALOGE("%s: error reading frame count tag: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        mParent->signalError();
        return false;
    }
    mNextFrameNumber = *e.data.i32;

    res = find_camera_metadata_entry(mRequest,
            ANDROID_SENSOR_EXPOSURE_TIME,
            &e);
    if (res != NO_ERROR) {
        ALOGE("%s: error reading exposure time tag: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        mParent->signalError();
        return false;
    }
    mNextExposureTime = *e.data.i64;

    res = find_camera_metadata_entry(mRequest,
            ANDROID_SENSOR_FRAME_DURATION,
            &e);
    if (res != NO_ERROR) {
        ALOGE("%s: error reading frame duration tag", __FUNCTION__);
        mParent->signalError();
        return false;
    }
    mNextFrameDuration = *e.data.i64;

    if (mNextFrameDuration <
            mNextExposureTime + Sensor::kMinVerticalBlank) {
        mNextFrameDuration = mNextExposureTime + Sensor::kMinVerticalBlank;
    }
    res = find_camera_metadata_entry(mRequest,
            ANDROID_SENSOR_SENSITIVITY,
            &e);
    if (res != NO_ERROR) {
        ALOGE("%s: error reading sensitivity tag", __FUNCTION__);
        mParent->signalError();
        return false;
    }
    mNextSensitivity = *e.data.i32;

    // Start waiting on readout thread
    mWaitingForReadout = true;
    ALOGV("Configure: Waiting for readout thread");

    return true;
}

bool EmulatedFakeCamera2::ConfigureThread::configureNextCapture() {
    bool vsync = mParent->mSensor->waitForVSync(kWaitPerLoop);
    if (!vsync) return true;

    Mutex::Autolock il(mInternalsMutex);
    ALOGV("Configure: Configuring sensor for capture %d", mNextFrameNumber);
    mParent->mSensor->setExposureTime(mNextExposureTime);
    mParent->mSensor->setFrameDuration(mNextFrameDuration);
    mParent->mSensor->setSensitivity(mNextSensitivity);

    getBuffers();

    ALOGV("Configure: Done configure for capture %d", mNextFrameNumber);
    mParent->mReadoutThread->setNextOperation(true, mRequest, mNextBuffers);
    mParent->mSensor->setDestinationBuffers(mNextBuffers);

    mRequest = NULL;
    mNextBuffers = NULL;

    Mutex::Autolock lock(mInputMutex);
    mRequestCount--;

    return true;
}

bool EmulatedFakeCamera2::ConfigureThread::setupReprocess() {
    status_t res;

    mNextNeedsJpeg = true;
    mNextIsCapture = false;

    camera_metadata_entry_t reprocessStreams;
    res = find_camera_metadata_entry(mRequest,
            ANDROID_REQUEST_INPUT_STREAMS,
            &reprocessStreams);
    if (res != NO_ERROR) {
        ALOGE("%s: error reading output stream tag", __FUNCTION__);
        mParent->signalError();
        return false;
    }

    mNextBuffers = new Buffers;

    ALOGV("Configure: Setting up input buffers for reprocess");
    for (size_t i = 0; i < reprocessStreams.count; i++) {
        int streamId = reprocessStreams.data.i32[i];
        const ReprocessStream &s = mParent->getReprocessStreamInfo(streamId);
        if (s.format != HAL_PIXEL_FORMAT_RGB_888) {
            ALOGE("%s: Only ZSL reprocessing supported!",
                    __FUNCTION__);
            mParent->signalError();
            return false;
        }
        StreamBuffer b;
        b.streamId = -streamId;
        b.width = s.width;
        b.height = s.height;
        b.format = s.format;
        b.stride = s.stride;
        mNextBuffers->push_back(b);
    }

    camera_metadata_entry_t streams;
    res = find_camera_metadata_entry(mRequest,
            ANDROID_REQUEST_OUTPUT_STREAMS,
            &streams);
    if (res != NO_ERROR) {
        ALOGE("%s: error reading output stream tag", __FUNCTION__);
        mParent->signalError();
        return false;
    }

    ALOGV("Configure: Setting up output buffers for reprocess");
    for (size_t i = 0; i < streams.count; i++) {
        int streamId = streams.data.i32[i];
        const Stream &s = mParent->getStreamInfo(streamId);
        if (s.format != HAL_PIXEL_FORMAT_BLOB) {
            // TODO: Support reprocess to YUV
            ALOGE("%s: Non-JPEG output stream %d for reprocess not supported",
                    __FUNCTION__, streamId);
            mParent->signalError();
            return false;
        }
        StreamBuffer b;
        b.streamId = streams.data.u8[i];
        b.width  = s.width;
        b.height = s.height;
        b.format = s.format;
        b.stride = s.stride;
        mNextBuffers->push_back(b);
        ALOGV("Configure:    Buffer %zu: Stream %d, %d x %d, format 0x%x, "
                "stride %d",
                i, b.streamId, b.width, b.height, b.format, b.stride);
    }

    camera_metadata_entry_t e;
    res = find_camera_metadata_entry(mRequest,
            ANDROID_REQUEST_FRAME_COUNT,
            &e);
    if (res != NO_ERROR) {
        ALOGE("%s: error reading frame count tag: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        mParent->signalError();
        return false;
    }
    mNextFrameNumber = *e.data.i32;

    return true;
}

bool EmulatedFakeCamera2::ConfigureThread::configureNextReprocess() {
    Mutex::Autolock il(mInternalsMutex);

    getBuffers();

    ALOGV("Configure: Done configure for reprocess %d", mNextFrameNumber);
    mParent->mReadoutThread->setNextOperation(false, mRequest, mNextBuffers);

    mRequest = NULL;
    mNextBuffers = NULL;

    Mutex::Autolock lock(mInputMutex);
    mRequestCount--;

    return true;
}

bool EmulatedFakeCamera2::ConfigureThread::getBuffers() {
    status_t res;
    /** Get buffers to fill for this frame */
    for (size_t i = 0; i < mNextBuffers->size(); i++) {
        StreamBuffer &b = mNextBuffers->editItemAt(i);

        if (b.streamId > 0) {
            Stream s = mParent->getStreamInfo(b.streamId);
            ALOGV("Configure: Dequeing buffer from stream %d", b.streamId);
            res = s.ops->dequeue_buffer(s.ops, &(b.buffer) );
            if (res != NO_ERROR || b.buffer == NULL) {
                ALOGE("%s: Unable to dequeue buffer from stream %d: %s (%d)",
                        __FUNCTION__, b.streamId, strerror(-res), res);
                mParent->signalError();
                return false;
            }

            /* Lock the buffer from the perspective of the graphics mapper */
            const Rect rect(s.width, s.height);

            res = GraphicBufferMapper::get().lock(*(b.buffer),
                    GRALLOC_USAGE_HW_CAMERA_WRITE,
                    rect, (void**)&(b.img) );

            if (res != NO_ERROR) {
                ALOGE("%s: grbuffer_mapper.lock failure: %s (%d)",
                        __FUNCTION__, strerror(-res), res);
                s.ops->cancel_buffer(s.ops,
                        b.buffer);
                mParent->signalError();
                return false;
            }
        } else {
            ReprocessStream s = mParent->getReprocessStreamInfo(-b.streamId);
            ALOGV("Configure: Acquiring buffer from reprocess stream %d",
                    -b.streamId);
            res = s.ops->acquire_buffer(s.ops, &(b.buffer) );
            if (res != NO_ERROR || b.buffer == NULL) {
                ALOGE("%s: Unable to acquire buffer from reprocess stream %d: "
                        "%s (%d)", __FUNCTION__, -b.streamId,
                        strerror(-res), res);
                mParent->signalError();
                return false;
            }

            /* Lock the buffer from the perspective of the graphics mapper */
            const Rect rect(s.width, s.height);

            res = GraphicBufferMapper::get().lock(*(b.buffer),
                    GRALLOC_USAGE_HW_CAMERA_READ,
                    rect, (void**)&(b.img) );
            if (res != NO_ERROR) {
                ALOGE("%s: grbuffer_mapper.lock failure: %s (%d)",
                        __FUNCTION__, strerror(-res), res);
                s.ops->release_buffer(s.ops,
                        b.buffer);
                mParent->signalError();
                return false;
            }
        }
    }
    return true;
}

EmulatedFakeCamera2::ReadoutThread::ReadoutThread(EmulatedFakeCamera2 *parent):
        Thread(false),
        mParent(parent),
        mRunning(false),
        mActive(false),
        mRequestCount(0),
        mRequest(NULL),
        mBuffers(NULL) {
    mInFlightQueue = new InFlightQueue[kInFlightQueueSize];
    mInFlightHead = 0;
    mInFlightTail = 0;
}

EmulatedFakeCamera2::ReadoutThread::~ReadoutThread() {
    delete mInFlightQueue;
}

status_t EmulatedFakeCamera2::ReadoutThread::readyToRun() {
    Mutex::Autolock lock(mInputMutex);
    ALOGV("Starting up ReadoutThread");
    mRunning = true;
    mInputSignal.signal();
    return NO_ERROR;
}

status_t EmulatedFakeCamera2::ReadoutThread::waitUntilRunning() {
    Mutex::Autolock lock(mInputMutex);
    if (!mRunning) {
        ALOGV("Waiting for readout thread to start");
        mInputSignal.wait(mInputMutex);
    }
    return OK;
}

bool EmulatedFakeCamera2::ReadoutThread::waitForReady(nsecs_t timeout) {
    status_t res;
    Mutex::Autolock lock(mInputMutex);
    while (!readyForNextCapture()) {
        res = mReadySignal.waitRelative(mInputMutex, timeout);
        if (res == TIMED_OUT) return false;
        if (res != OK) {
            ALOGE("%s: Error waiting for ready: %s (%d)", __FUNCTION__,
                    strerror(-res), res);
            return false;
        }
    }
    return true;
}

bool EmulatedFakeCamera2::ReadoutThread::readyForNextCapture() {
    return (mInFlightTail + 1) % kInFlightQueueSize != mInFlightHead;
}

void EmulatedFakeCamera2::ReadoutThread::setNextOperation(
        bool isCapture,
        camera_metadata_t *request,
        Buffers *buffers) {
    Mutex::Autolock lock(mInputMutex);
    if ( !readyForNextCapture() ) {
        ALOGE("In flight queue full, dropping captures");
        mParent->signalError();
        return;
    }
    mInFlightQueue[mInFlightTail].isCapture = isCapture;
    mInFlightQueue[mInFlightTail].request = request;
    mInFlightQueue[mInFlightTail].buffers = buffers;
    mInFlightTail = (mInFlightTail + 1) % kInFlightQueueSize;
    mRequestCount++;

    if (!mActive) {
        mActive = true;
        mInputSignal.signal();
    }
}

bool EmulatedFakeCamera2::ReadoutThread::isStreamInUse(uint32_t id) {
    // acquire in same order as threadLoop
    Mutex::Autolock iLock(mInternalsMutex);
    Mutex::Autolock lock(mInputMutex);

    size_t i = mInFlightHead;
    while (i != mInFlightTail) {
        for (size_t j = 0; j < mInFlightQueue[i].buffers->size(); j++) {
            if ( (*(mInFlightQueue[i].buffers))[j].streamId == (int)id )
                return true;
        }
        i = (i + 1) % kInFlightQueueSize;
    }


    if (mBuffers != NULL) {
        for (i = 0; i < mBuffers->size(); i++) {
            if ( (*mBuffers)[i].streamId == (int)id) return true;
        }
    }

    return false;
}

int EmulatedFakeCamera2::ReadoutThread::getInProgressCount() {
    Mutex::Autolock lock(mInputMutex);

    return mRequestCount;
}

bool EmulatedFakeCamera2::ReadoutThread::threadLoop() {
    static const nsecs_t kWaitPerLoop = 10000000L; // 10 ms
    status_t res;
    int32_t frameNumber;

    // Check if we're currently processing or just waiting
    {
        Mutex::Autolock lock(mInputMutex);
        if (!mActive) {
            // Inactive, keep waiting until we've been signaled
            res = mInputSignal.waitRelative(mInputMutex, kWaitPerLoop);
            if (res != NO_ERROR && res != TIMED_OUT) {
                ALOGE("%s: Error waiting for capture requests: %d",
                        __FUNCTION__, res);
                mParent->signalError();
                return false;
            }
            if (!mActive) return true;
        }
        // Active, see if we need a new request
        if (mRequest == NULL) {
            if (mInFlightHead == mInFlightTail) {
                // Go inactive
                ALOGV("Waiting for sensor data");
                mActive = false;
                return true;
            } else {
                Mutex::Autolock iLock(mInternalsMutex);
                mReadySignal.signal();
                mIsCapture = mInFlightQueue[mInFlightHead].isCapture;
                mRequest = mInFlightQueue[mInFlightHead].request;
                mBuffers  = mInFlightQueue[mInFlightHead].buffers;
                mInFlightQueue[mInFlightHead].request = NULL;
                mInFlightQueue[mInFlightHead].buffers = NULL;
                mInFlightHead = (mInFlightHead + 1) % kInFlightQueueSize;
                ALOGV("Ready to read out request %p, %zu buffers",
                        mRequest, mBuffers->size());
            }
        }
    }

    // Active with request, wait on sensor to complete

    nsecs_t captureTime;

    if (mIsCapture) {
        bool gotFrame;
        gotFrame = mParent->mSensor->waitForNewFrame(kWaitPerLoop,
                &captureTime);

        if (!gotFrame) return true;
    }

    Mutex::Autolock iLock(mInternalsMutex);

    camera_metadata_entry_t entry;
    if (!mIsCapture) {
        res = find_camera_metadata_entry(mRequest,
                ANDROID_SENSOR_TIMESTAMP,
            &entry);
        if (res != NO_ERROR) {
            ALOGE("%s: error reading reprocessing timestamp: %s (%d)",
                    __FUNCTION__, strerror(-res), res);
            mParent->signalError();
            return false;
        }
        captureTime = entry.data.i64[0];
    }

    res = find_camera_metadata_entry(mRequest,
            ANDROID_REQUEST_FRAME_COUNT,
            &entry);
    if (res != NO_ERROR) {
        ALOGE("%s: error reading frame count tag: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        mParent->signalError();
        return false;
    }
    frameNumber = *entry.data.i32;

    res = find_camera_metadata_entry(mRequest,
            ANDROID_REQUEST_METADATA_MODE,
            &entry);
    if (res != NO_ERROR) {
        ALOGE("%s: error reading metadata mode tag: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        mParent->signalError();
        return false;
    }

    // Got sensor data and request, construct frame and send it out
    ALOGV("Readout: Constructing metadata and frames for request %d",
            frameNumber);

    if (*entry.data.u8 == ANDROID_REQUEST_METADATA_MODE_FULL) {
        ALOGV("Readout: Metadata requested, constructing");

        camera_metadata_t *frame = NULL;

        size_t frame_entries = get_camera_metadata_entry_count(mRequest);
        size_t frame_data    = get_camera_metadata_data_count(mRequest);

        // TODO: Dynamically calculate based on enabled statistics, etc
        frame_entries += 10;
        frame_data += 100;

        res = mParent->mFrameQueueDst->dequeue_frame(mParent->mFrameQueueDst,
                frame_entries, frame_data, &frame);

        if (res != NO_ERROR || frame == NULL) {
            ALOGE("%s: Unable to dequeue frame metadata buffer", __FUNCTION__);
            mParent->signalError();
            return false;
        }

        res = append_camera_metadata(frame, mRequest);
        if (res != NO_ERROR) {
            ALOGE("Unable to append request metadata");
        }

        if (mIsCapture) {
            add_camera_metadata_entry(frame,
                    ANDROID_SENSOR_TIMESTAMP,
                    &captureTime,
                    1);

            collectStatisticsMetadata(frame);
            // TODO: Collect all final values used from sensor in addition to timestamp
        }

        ALOGV("Readout: Enqueue frame %d", frameNumber);
        mParent->mFrameQueueDst->enqueue_frame(mParent->mFrameQueueDst,
                frame);
    }
    ALOGV("Readout: Free request");
    res = mParent->mRequestQueueSrc->free_request(mParent->mRequestQueueSrc, mRequest);
    if (res != NO_ERROR) {
        ALOGE("%s: Unable to return request buffer to queue: %d",
                __FUNCTION__, res);
        mParent->signalError();
        return false;
    }
    mRequest = NULL;

    int compressedBufferIndex = -1;
    ALOGV("Readout: Processing %zu buffers", mBuffers->size());
    for (size_t i = 0; i < mBuffers->size(); i++) {
        const StreamBuffer &b = (*mBuffers)[i];
        ALOGV("Readout:    Buffer %zu: Stream %d, %d x %d, format 0x%x, stride %d",
                i, b.streamId, b.width, b.height, b.format, b.stride);
        if (b.streamId > 0) {
            if (b.format == HAL_PIXEL_FORMAT_BLOB) {
                // Assumes only one BLOB buffer type per capture
                compressedBufferIndex = i;
            } else {
                ALOGV("Readout:    Sending image buffer %zu (%p) to output stream %d",
                        i, (void*)*(b.buffer), b.streamId);
                GraphicBufferMapper::get().unlock(*(b.buffer));
                const Stream &s = mParent->getStreamInfo(b.streamId);
                res = s.ops->enqueue_buffer(s.ops, captureTime, b.buffer);
                if (res != OK) {
                    ALOGE("Error enqueuing image buffer %p: %s (%d)", b.buffer,
                            strerror(-res), res);
                    mParent->signalError();
                }
            }
        }
    }

    if (compressedBufferIndex == -1) {
        delete mBuffers;
    } else {
        ALOGV("Readout:  Starting JPEG compression for buffer %d, stream %d",
                compressedBufferIndex,
                (*mBuffers)[compressedBufferIndex].streamId);
        mJpegTimestamp = captureTime;
        // Takes ownership of mBuffers
        mParent->mJpegCompressor->start(mBuffers, this);
    }
    mBuffers = NULL;

    Mutex::Autolock l(mInputMutex);
    mRequestCount--;
    ALOGV("Readout: Done with request %d", frameNumber);
    return true;
}

void EmulatedFakeCamera2::ReadoutThread::onJpegDone(
        const StreamBuffer &jpegBuffer, bool success) {
    status_t res;
    if (!success) {
        ALOGE("%s: Error queueing compressed image buffer %p",
                __FUNCTION__, jpegBuffer.buffer);
        mParent->signalError();
        return;
    }

    // Write to JPEG output stream
    ALOGV("%s: Compression complete, pushing to stream %d", __FUNCTION__,
            jpegBuffer.streamId);

    GraphicBufferMapper::get().unlock(*(jpegBuffer.buffer));
    const Stream &s = mParent->getStreamInfo(jpegBuffer.streamId);
    res = s.ops->enqueue_buffer(s.ops, mJpegTimestamp, jpegBuffer.buffer);
}

void EmulatedFakeCamera2::ReadoutThread::onJpegInputDone(
        const StreamBuffer &inputBuffer) {
    status_t res;
    GraphicBufferMapper::get().unlock(*(inputBuffer.buffer));
    const ReprocessStream &s =
            mParent->getReprocessStreamInfo(-inputBuffer.streamId);
    res = s.ops->release_buffer(s.ops, inputBuffer.buffer);
    if (res != OK) {
        ALOGE("Error releasing reprocess buffer %p: %s (%d)",
                inputBuffer.buffer, strerror(-res), res);
        mParent->signalError();
    }
}

status_t EmulatedFakeCamera2::ReadoutThread::collectStatisticsMetadata(
        camera_metadata_t *frame) {
    // Completely fake face rectangles, don't correspond to real faces in scene
    ALOGV("Readout:    Collecting statistics metadata");

    status_t res;
    camera_metadata_entry_t entry;
    res = find_camera_metadata_entry(frame,
                ANDROID_STATISTICS_FACE_DETECT_MODE,
                &entry);
    if (res != OK) {
        ALOGE("%s: Unable to find face detect mode!", __FUNCTION__);
        return BAD_VALUE;
    }

    if (entry.data.u8[0] == ANDROID_STATISTICS_FACE_DETECT_MODE_OFF) return OK;

    // The coordinate system for the face regions is the raw sensor pixel
    // coordinates. Here, we map from the scene coordinates (0-19 in both axis)
    // to raw pixels, for the scene defined in fake-pipeline2/Scene.cpp. We
    // approximately place two faces on top of the windows of the house. No
    // actual faces exist there, but might one day. Note that this doesn't
    // account for the offsets used to account for aspect ratio differences, so
    // the rectangles don't line up quite right.
    const size_t numFaces = 2;
    int32_t rects[numFaces * 4] = {
        static_cast<int32_t>(Sensor::kResolution[0] * 10 / 20),
        static_cast<int32_t>(Sensor::kResolution[1] * 15 / 20),
        static_cast<int32_t>(Sensor::kResolution[0] * 12 / 20),
        static_cast<int32_t>(Sensor::kResolution[1] * 17 / 20),

        static_cast<int32_t>(Sensor::kResolution[0] * 16 / 20),
        static_cast<int32_t>(Sensor::kResolution[1] * 15 / 20),
        static_cast<int32_t>(Sensor::kResolution[0] * 18 / 20),
        static_cast<int32_t>(Sensor::kResolution[1] * 17 / 20)
    };
    // To simulate some kind of real detection going on, we jitter the rectangles on
    // each frame by a few pixels in each dimension.
    for (size_t i = 0; i < numFaces * 4; i++) {
        rects[i] += (int32_t)(((float)rand() / RAND_MAX) * 6 - 3);
    }
    // The confidence scores (0-100) are similarly jittered.
    uint8_t scores[numFaces] = { 85, 95 };
    for (size_t i = 0; i < numFaces; i++) {
        scores[i] += (int32_t)(((float)rand() / RAND_MAX) * 10 - 5);
    }

    res = add_camera_metadata_entry(frame, ANDROID_STATISTICS_FACE_RECTANGLES,
            rects, numFaces * 4);
    if (res != OK) {
        ALOGE("%s: Unable to add face rectangles!", __FUNCTION__);
        return BAD_VALUE;
    }

    res = add_camera_metadata_entry(frame, ANDROID_STATISTICS_FACE_SCORES,
            scores, numFaces);
    if (res != OK) {
        ALOGE("%s: Unable to add face scores!", __FUNCTION__);
        return BAD_VALUE;
    }

    if (entry.data.u8[0] == ANDROID_STATISTICS_FACE_DETECT_MODE_SIMPLE) return OK;

    // Advanced face detection options - add eye/mouth coordinates.  The
    // coordinates in order are (leftEyeX, leftEyeY, rightEyeX, rightEyeY,
    // mouthX, mouthY). The mapping is the same as the face rectangles.
    int32_t features[numFaces * 6] = {
        static_cast<int32_t>(Sensor::kResolution[0] * 10.5 / 20),
        static_cast<int32_t>(Sensor::kResolution[1] * 16 / 20),
        static_cast<int32_t>(Sensor::kResolution[0] * 11.5 / 20),
        static_cast<int32_t>(Sensor::kResolution[1] * 16 / 20),
        static_cast<int32_t>(Sensor::kResolution[0] * 11 / 20),
        static_cast<int32_t>(Sensor::kResolution[1] * 16.5 / 20),

        static_cast<int32_t>(Sensor::kResolution[0] * 16.5 / 20),
        static_cast<int32_t>(Sensor::kResolution[1] * 16 / 20),
        static_cast<int32_t>(Sensor::kResolution[0] * 17.5 / 20),
        static_cast<int32_t>(Sensor::kResolution[1] * 16 / 20),
        static_cast<int32_t>(Sensor::kResolution[0] * 17 / 20),
        static_cast<int32_t>(Sensor::kResolution[1] * 16.5 / 20),
    };
    // Jitter these a bit less than the rects
    for (size_t i = 0; i < numFaces * 6; i++) {
        features[i] += (int32_t)(((float)rand() / RAND_MAX) * 4 - 2);
    }
    // These are unique IDs that are used to identify each face while it's
    // visible to the detector (if a face went away and came back, it'd get a
    // new ID).
    int32_t ids[numFaces] = {
        100, 200
    };

    res = add_camera_metadata_entry(frame, ANDROID_STATISTICS_FACE_LANDMARKS,
            features, numFaces * 6);
    if (res != OK) {
        ALOGE("%s: Unable to add face landmarks!", __FUNCTION__);
        return BAD_VALUE;
    }

    res = add_camera_metadata_entry(frame, ANDROID_STATISTICS_FACE_IDS,
            ids, numFaces);
    if (res != OK) {
        ALOGE("%s: Unable to add face scores!", __FUNCTION__);
        return BAD_VALUE;
    }

    return OK;
}

EmulatedFakeCamera2::ControlThread::ControlThread(EmulatedFakeCamera2 *parent):
        Thread(false),
        mParent(parent) {
    mRunning = false;
}

EmulatedFakeCamera2::ControlThread::~ControlThread() {
}

status_t EmulatedFakeCamera2::ControlThread::readyToRun() {
    Mutex::Autolock lock(mInputMutex);

    ALOGV("Starting up ControlThread");
    mRunning = true;
    mStartAf = false;
    mCancelAf = false;
    mStartPrecapture = false;

    mControlMode = ANDROID_CONTROL_MODE_AUTO;

    mEffectMode = ANDROID_CONTROL_EFFECT_MODE_OFF;
    mSceneMode = ANDROID_CONTROL_SCENE_MODE_FACE_PRIORITY;

    mAfMode = ANDROID_CONTROL_AF_MODE_AUTO;
    mAfModeChange = false;

    mAeMode = ANDROID_CONTROL_AE_MODE_ON;
    mAwbMode = ANDROID_CONTROL_AWB_MODE_AUTO;

    mAfTriggerId = 0;
    mPrecaptureTriggerId = 0;

    mAfState = ANDROID_CONTROL_AF_STATE_INACTIVE;
    mAeState = ANDROID_CONTROL_AE_STATE_INACTIVE;
    mAwbState = ANDROID_CONTROL_AWB_STATE_INACTIVE;

    mExposureTime = kNormalExposureTime;

    mInputSignal.signal();
    return NO_ERROR;
}

status_t EmulatedFakeCamera2::ControlThread::waitUntilRunning() {
    Mutex::Autolock lock(mInputMutex);
    if (!mRunning) {
        ALOGV("Waiting for control thread to start");
        mInputSignal.wait(mInputMutex);
    }
    return OK;
}

// Override android.control.* fields with 3A values before sending request to sensor
status_t EmulatedFakeCamera2::ControlThread::processRequest(camera_metadata_t *request) {
    Mutex::Autolock lock(mInputMutex);
    // TODO: Add handling for all android.control.* fields here
    camera_metadata_entry_t mode;
    status_t res;

#define READ_IF_OK(res, what, def)                                             \
    (((res) == OK) ? (what) : (uint8_t)(def))

    res = find_camera_metadata_entry(request,
            ANDROID_CONTROL_MODE,
            &mode);
    mControlMode = READ_IF_OK(res, mode.data.u8[0], ANDROID_CONTROL_MODE_OFF);

    // disable all 3A
    if (mControlMode == ANDROID_CONTROL_MODE_OFF) {
        mEffectMode =   ANDROID_CONTROL_EFFECT_MODE_OFF;
        mSceneMode =    ANDROID_CONTROL_SCENE_MODE_DISABLED;
        mAfMode =       ANDROID_CONTROL_AF_MODE_OFF;
        mAeLock =       ANDROID_CONTROL_AE_LOCK_ON;
        mAeMode =       ANDROID_CONTROL_AE_MODE_OFF;
        mAfModeChange = true;
        mStartAf =      false;
        mCancelAf =     true;
        mAeState =      ANDROID_CONTROL_AE_STATE_INACTIVE;
        mAwbMode =      ANDROID_CONTROL_AWB_MODE_OFF;
        return res;
    }

    res = find_camera_metadata_entry(request,
            ANDROID_CONTROL_EFFECT_MODE,
            &mode);
    mEffectMode = READ_IF_OK(res, mode.data.u8[0],
                             ANDROID_CONTROL_EFFECT_MODE_OFF);

    res = find_camera_metadata_entry(request,
            ANDROID_CONTROL_SCENE_MODE,
            &mode);
    mSceneMode = READ_IF_OK(res, mode.data.u8[0],
                             ANDROID_CONTROL_SCENE_MODE_DISABLED);

    res = find_camera_metadata_entry(request,
            ANDROID_CONTROL_AF_MODE,
            &mode);
    if (mAfMode != mode.data.u8[0]) {
        ALOGV("AF new mode: %d, old mode %d", mode.data.u8[0], mAfMode);
        mAfMode = mode.data.u8[0];
        mAfModeChange = true;
        mStartAf = false;
        mCancelAf = false;
    }

    res = find_camera_metadata_entry(request,
            ANDROID_CONTROL_AE_MODE,
            &mode);
    mAeMode = READ_IF_OK(res, mode.data.u8[0],
                             ANDROID_CONTROL_AE_MODE_OFF);

    res = find_camera_metadata_entry(request,
            ANDROID_CONTROL_AE_LOCK,
            &mode);
    uint8_t aeLockVal = READ_IF_OK(res, mode.data.u8[0],
                                   ANDROID_CONTROL_AE_LOCK_ON);
    bool aeLock = (aeLockVal == ANDROID_CONTROL_AE_LOCK_ON);
    if (mAeLock && !aeLock) {
        mAeState = ANDROID_CONTROL_AE_STATE_INACTIVE;
    }
    mAeLock = aeLock;

    res = find_camera_metadata_entry(request,
            ANDROID_CONTROL_AWB_MODE,
            &mode);
    mAwbMode = READ_IF_OK(res, mode.data.u8[0],
                          ANDROID_CONTROL_AWB_MODE_OFF);

    // TODO: Override more control fields

    if (mAeMode != ANDROID_CONTROL_AE_MODE_OFF) {
        camera_metadata_entry_t exposureTime;
        res = find_camera_metadata_entry(request,
                ANDROID_SENSOR_EXPOSURE_TIME,
                &exposureTime);
        if (res == OK) {
            exposureTime.data.i64[0] = mExposureTime;
        }
    }

#undef READ_IF_OK

    return OK;
}

status_t EmulatedFakeCamera2::ControlThread::triggerAction(uint32_t msgType,
        int32_t ext1, int32_t ext2) {
    ALOGV("%s: Triggering %d (%d, %d)", __FUNCTION__, msgType, ext1, ext2);
    Mutex::Autolock lock(mInputMutex);
    switch (msgType) {
        case CAMERA2_TRIGGER_AUTOFOCUS:
            mAfTriggerId = ext1;
            mStartAf = true;
            mCancelAf = false;
            break;
        case CAMERA2_TRIGGER_CANCEL_AUTOFOCUS:
            mAfTriggerId = ext1;
            mStartAf = false;
            mCancelAf = true;
            break;
        case CAMERA2_TRIGGER_PRECAPTURE_METERING:
            mPrecaptureTriggerId = ext1;
            mStartPrecapture = true;
            break;
        default:
            ALOGE("%s: Unknown action triggered: %d (arguments %d %d)",
                    __FUNCTION__, msgType, ext1, ext2);
            return BAD_VALUE;
    }
    return OK;
}

const nsecs_t EmulatedFakeCamera2::ControlThread::kControlCycleDelay = 100 * MSEC;
const nsecs_t EmulatedFakeCamera2::ControlThread::kMinAfDuration = 500 * MSEC;
const nsecs_t EmulatedFakeCamera2::ControlThread::kMaxAfDuration = 900 * MSEC;
const float EmulatedFakeCamera2::ControlThread::kAfSuccessRate = 0.9;
 // Once every 5 seconds
const float EmulatedFakeCamera2::ControlThread::kContinuousAfStartRate =
        kControlCycleDelay / 5.0 * SEC;
const nsecs_t EmulatedFakeCamera2::ControlThread::kMinAeDuration = 500 * MSEC;
const nsecs_t EmulatedFakeCamera2::ControlThread::kMaxAeDuration = 2 * SEC;
const nsecs_t EmulatedFakeCamera2::ControlThread::kMinPrecaptureAeDuration = 100 * MSEC;
const nsecs_t EmulatedFakeCamera2::ControlThread::kMaxPrecaptureAeDuration = 400 * MSEC;
 // Once every 3 seconds
const float EmulatedFakeCamera2::ControlThread::kAeScanStartRate =
    kControlCycleDelay / 3000000000.0;

const nsecs_t EmulatedFakeCamera2::ControlThread::kNormalExposureTime = 10 * MSEC;
const nsecs_t EmulatedFakeCamera2::ControlThread::kExposureJump = 2 * MSEC;
const nsecs_t EmulatedFakeCamera2::ControlThread::kMinExposureTime = 1 * MSEC;

bool EmulatedFakeCamera2::ControlThread::threadLoop() {
    bool afModeChange = false;
    bool afTriggered = false;
    bool afCancelled = false;
    uint8_t afState;
    uint8_t afMode;
    int32_t afTriggerId;
    bool precaptureTriggered = false;
    uint8_t aeState;
    uint8_t aeMode;
    bool    aeLock;
    int32_t precaptureTriggerId;
    nsecs_t nextSleep = kControlCycleDelay;

    {
        Mutex::Autolock lock(mInputMutex);
        if (mStartAf) {
            ALOGD("Starting AF trigger processing");
            afTriggered = true;
            mStartAf = false;
        } else if (mCancelAf) {
            ALOGD("Starting cancel AF trigger processing");
            afCancelled = true;
            mCancelAf = false;
        }
        afState = mAfState;
        afMode = mAfMode;
        afModeChange = mAfModeChange;
        mAfModeChange = false;

        afTriggerId = mAfTriggerId;

        if(mStartPrecapture) {
            ALOGD("Starting precapture trigger processing");
            precaptureTriggered = true;
            mStartPrecapture = false;
        }
        aeState = mAeState;
        aeMode = mAeMode;
        aeLock = mAeLock;
        precaptureTriggerId = mPrecaptureTriggerId;
    }

    if (afCancelled || afModeChange) {
        ALOGV("Resetting AF state due to cancel/mode change");
        afState = ANDROID_CONTROL_AF_STATE_INACTIVE;
        updateAfState(afState, afTriggerId);
        mAfScanDuration = 0;
        mLockAfterPassiveScan = false;
    }

    uint8_t oldAfState = afState;

    if (afTriggered) {
        afState = processAfTrigger(afMode, afState);
    }

    afState = maybeStartAfScan(afMode, afState);
    afState = updateAfScan(afMode, afState, &nextSleep);
    updateAfState(afState, afTriggerId);

    if (precaptureTriggered) {
        aeState = processPrecaptureTrigger(aeMode, aeState);
    }

    aeState = maybeStartAeScan(aeMode, aeLock, aeState);
    aeState = updateAeScan(aeMode, aeLock, aeState, &nextSleep);
    updateAeState(aeState, precaptureTriggerId);

    int ret;
    timespec t;
    t.tv_sec = 0;
    t.tv_nsec = nextSleep;
    do {
        ret = nanosleep(&t, &t);
    } while (ret != 0);

    if (mAfScanDuration > 0) {
        mAfScanDuration -= nextSleep;
    }
    if (mAeScanDuration > 0) {
        mAeScanDuration -= nextSleep;
    }

    return true;
}

int EmulatedFakeCamera2::ControlThread::processAfTrigger(uint8_t afMode,
        uint8_t afState) {
    switch (afMode) {
        case ANDROID_CONTROL_AF_MODE_OFF:
        case ANDROID_CONTROL_AF_MODE_EDOF:
            // Do nothing
            break;
        case ANDROID_CONTROL_AF_MODE_MACRO:
        case ANDROID_CONTROL_AF_MODE_AUTO:
            switch (afState) {
                case ANDROID_CONTROL_AF_STATE_INACTIVE:
                case ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED:
                case ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:
                    // Start new focusing cycle
                    mAfScanDuration =  ((double)rand() / RAND_MAX) *
                        (kMaxAfDuration - kMinAfDuration) + kMinAfDuration;
                    afState = ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN;
                    ALOGV("%s: AF scan start, duration %" PRId64 " ms",
                          __FUNCTION__, mAfScanDuration / 1000000);
                    break;
                case ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN:
                    // Ignore new request, already scanning
                    break;
                default:
                    ALOGE("Unexpected AF state in AUTO/MACRO AF mode: %d",
                          afState);
            }
            break;
        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
            switch (afState) {
                // Picture mode waits for passive scan to complete
                case ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN:
                    mLockAfterPassiveScan = true;
                    break;
                case ANDROID_CONTROL_AF_STATE_INACTIVE:
                    afState = ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
                    break;
                case ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED:
                    afState = ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED;
                    break;
                case ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED:
                case ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:
                    // Must cancel to get out of these states
                    break;
                default:
                    ALOGE("Unexpected AF state in CONTINUOUS_PICTURE AF mode: %d",
                          afState);
            }
            break;
        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
            switch (afState) {
                // Video mode does not wait for passive scan to complete
                case ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN:
                case ANDROID_CONTROL_AF_STATE_INACTIVE:
                    afState = ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
                    break;
                case ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED:
                    afState = ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED;
                    break;
                case ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED:
                case ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED:
                    // Must cancel to get out of these states
                    break;
                default:
                    ALOGE("Unexpected AF state in CONTINUOUS_VIDEO AF mode: %d",
                          afState);
            }
            break;
        default:
            break;
    }
    return afState;
}

int EmulatedFakeCamera2::ControlThread::maybeStartAfScan(uint8_t afMode,
        uint8_t afState) {
    if ((afMode == ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO ||
            afMode == ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE) &&
        (afState == ANDROID_CONTROL_AF_STATE_INACTIVE ||
            afState == ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED)) {

        bool startScan = ((double)rand() / RAND_MAX) < kContinuousAfStartRate;
        if (startScan) {
            // Start new passive focusing cycle
            mAfScanDuration =  ((double)rand() / RAND_MAX) *
                (kMaxAfDuration - kMinAfDuration) + kMinAfDuration;
            afState = ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN;
            ALOGV("%s: AF passive scan start, duration %" PRId64 " ms",
                __FUNCTION__, mAfScanDuration / 1000000);
        }
    }
    return afState;
}

int EmulatedFakeCamera2::ControlThread::updateAfScan(uint8_t afMode,
        uint8_t afState, nsecs_t *maxSleep) {
    if (! (afState == ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN ||
            afState == ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN ) ) {
        return afState;
    }

    if (mAfScanDuration <= 0) {
        ALOGV("%s: AF scan done", __FUNCTION__);
        switch (afMode) {
            case ANDROID_CONTROL_AF_MODE_MACRO:
            case ANDROID_CONTROL_AF_MODE_AUTO: {
                bool success = ((double)rand() / RAND_MAX) < kAfSuccessRate;
                if (success) {
                    afState = ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED;
                } else {
                    afState = ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
                }
                break;
            }
            case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
                if (mLockAfterPassiveScan) {
                    afState = ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED;
                    mLockAfterPassiveScan = false;
                } else {
                    afState = ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED;
                }
                break;
            case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
                afState = ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED;
                break;
            default:
                ALOGE("Unexpected AF mode in scan state");
        }
    } else {
        if (mAfScanDuration <= *maxSleep) {
            *maxSleep = mAfScanDuration;
        }
    }
    return afState;
}

void EmulatedFakeCamera2::ControlThread::updateAfState(uint8_t newState,
        int32_t triggerId) {
    Mutex::Autolock lock(mInputMutex);
    if (mAfState != newState) {
        ALOGV("%s: Autofocus state now %d, id %d", __FUNCTION__,
                newState, triggerId);
        mAfState = newState;
        mParent->sendNotification(CAMERA2_MSG_AUTOFOCUS,
                newState, triggerId, 0);
    }
}

int EmulatedFakeCamera2::ControlThread::processPrecaptureTrigger(uint8_t aeMode,
        uint8_t aeState) {
    switch (aeMode) {
        case ANDROID_CONTROL_AE_MODE_OFF:
            // Don't do anything for these
            return aeState;
        case ANDROID_CONTROL_AE_MODE_ON:
        case ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH:
        case ANDROID_CONTROL_AE_MODE_ON_ALWAYS_FLASH:
        case ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE:
            // Trigger a precapture cycle
            aeState = ANDROID_CONTROL_AE_STATE_PRECAPTURE;
            mAeScanDuration = ((double)rand() / RAND_MAX) *
                    (kMaxPrecaptureAeDuration - kMinPrecaptureAeDuration) +
                    kMinPrecaptureAeDuration;
            ALOGD("%s: AE precapture scan start, duration %" PRId64 " ms",
                    __FUNCTION__, mAeScanDuration / 1000000);

    }
    return aeState;
}

int EmulatedFakeCamera2::ControlThread::maybeStartAeScan(uint8_t aeMode,
        bool aeLocked,
        uint8_t aeState) {
    if (aeLocked) return aeState;
    switch (aeMode) {
        case ANDROID_CONTROL_AE_MODE_OFF:
            break;
        case ANDROID_CONTROL_AE_MODE_ON:
        case ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH:
        case ANDROID_CONTROL_AE_MODE_ON_ALWAYS_FLASH:
        case ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE: {
            if (aeState != ANDROID_CONTROL_AE_STATE_INACTIVE &&
                    aeState != ANDROID_CONTROL_AE_STATE_CONVERGED) break;

            bool startScan = ((double)rand() / RAND_MAX) < kAeScanStartRate;
            if (startScan) {
                mAeScanDuration = ((double)rand() / RAND_MAX) *
                (kMaxAeDuration - kMinAeDuration) + kMinAeDuration;
                aeState = ANDROID_CONTROL_AE_STATE_SEARCHING;
                ALOGV("%s: AE scan start, duration %" PRId64 " ms",
                        __FUNCTION__, mAeScanDuration / 1000000);
            }
        }
    }

    return aeState;
}

int EmulatedFakeCamera2::ControlThread::updateAeScan(uint8_t aeMode,
        bool aeLock, uint8_t aeState, nsecs_t *maxSleep) {
    if (aeLock && aeState != ANDROID_CONTROL_AE_STATE_PRECAPTURE) {
        mAeScanDuration = 0;
        aeState = ANDROID_CONTROL_AE_STATE_LOCKED;
    } else if ((aeState == ANDROID_CONTROL_AE_STATE_SEARCHING) ||
            (aeState == ANDROID_CONTROL_AE_STATE_PRECAPTURE ) ) {
        if (mAeScanDuration <= 0) {
            ALOGV("%s: AE scan done", __FUNCTION__);
            aeState = aeLock ?
                    ANDROID_CONTROL_AE_STATE_LOCKED :ANDROID_CONTROL_AE_STATE_CONVERGED;

            Mutex::Autolock lock(mInputMutex);
            mExposureTime = kNormalExposureTime;
        } else {
            if (mAeScanDuration <= *maxSleep) {
                *maxSleep = mAeScanDuration;
            }

            int64_t exposureDelta =
                    ((double)rand() / RAND_MAX) * 2 * kExposureJump -
                    kExposureJump;
            Mutex::Autolock lock(mInputMutex);
            mExposureTime = mExposureTime + exposureDelta;
            if (mExposureTime < kMinExposureTime) mExposureTime = kMinExposureTime;
        }
    }

    return aeState;
}


void EmulatedFakeCamera2::ControlThread::updateAeState(uint8_t newState,
        int32_t triggerId) {
    Mutex::Autolock lock(mInputMutex);
    if (mAeState != newState) {
        ALOGV("%s: Autoexposure state now %d, id %d", __FUNCTION__,
                newState, triggerId);
        mAeState = newState;
        mParent->sendNotification(CAMERA2_MSG_AUTOEXPOSURE,
                newState, triggerId, 0);
    }
}

/** Private methods */

status_t EmulatedFakeCamera2::constructStaticInfo(
        camera_metadata_t **info,
        bool sizeRequest) const {

    size_t entryCount = 0;
    size_t dataCount = 0;
    status_t ret;

#define ADD_OR_SIZE( tag, data, count ) \
    if ( ( ret = addOrSize(*info, sizeRequest, &entryCount, &dataCount, \
            tag, data, count) ) != OK ) return ret

    // android.lens

    // 5 cm min focus distance for back camera, infinity (fixed focus) for front
    const float minFocusDistance = mFacingBack ? 1.0/0.05 : 0.0;
    ADD_OR_SIZE(ANDROID_LENS_INFO_MINIMUM_FOCUS_DISTANCE,
            &minFocusDistance, 1);
    // 5 m hyperfocal distance for back camera, infinity (fixed focus) for front
    const float hyperFocalDistance = mFacingBack ? 1.0/5.0 : 0.0;
    ADD_OR_SIZE(ANDROID_LENS_INFO_HYPERFOCAL_DISTANCE,
            &minFocusDistance, 1);

    static const float focalLength = 3.30f; // mm
    ADD_OR_SIZE(ANDROID_LENS_INFO_AVAILABLE_FOCAL_LENGTHS,
            &focalLength, 1);
    static const float aperture = 2.8f;
    ADD_OR_SIZE(ANDROID_LENS_INFO_AVAILABLE_APERTURES,
            &aperture, 1);
    static const float filterDensity = 0;
    ADD_OR_SIZE(ANDROID_LENS_INFO_AVAILABLE_FILTER_DENSITIES,
            &filterDensity, 1);
    static const uint8_t availableOpticalStabilization =
            ANDROID_LENS_OPTICAL_STABILIZATION_MODE_OFF;
    ADD_OR_SIZE(ANDROID_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION,
            &availableOpticalStabilization, 1);

    static const int32_t lensShadingMapSize[] = {1, 1};
    ADD_OR_SIZE(ANDROID_LENS_INFO_SHADING_MAP_SIZE, lensShadingMapSize,
            sizeof(lensShadingMapSize)/sizeof(int32_t));

    int32_t lensFacing = mFacingBack ?
            ANDROID_LENS_FACING_BACK : ANDROID_LENS_FACING_FRONT;
    ADD_OR_SIZE(ANDROID_LENS_FACING, &lensFacing, 1);

    // android.sensor

    ADD_OR_SIZE(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE,
            Sensor::kExposureTimeRange, 2);

    ADD_OR_SIZE(ANDROID_SENSOR_INFO_MAX_FRAME_DURATION,
            &Sensor::kFrameDurationRange[1], 1);

    ADD_OR_SIZE(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE,
            Sensor::kSensitivityRange,
            sizeof(Sensor::kSensitivityRange)
            /sizeof(int32_t));

    ADD_OR_SIZE(ANDROID_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT,
            &Sensor::kColorFilterArrangement, 1);

    static const float sensorPhysicalSize[2] = {3.20f, 2.40f}; // mm
    ADD_OR_SIZE(ANDROID_SENSOR_INFO_PHYSICAL_SIZE,
            sensorPhysicalSize, 2);

    ADD_OR_SIZE(ANDROID_SENSOR_INFO_PIXEL_ARRAY_SIZE,
            Sensor::kResolution, 2);

    ADD_OR_SIZE(ANDROID_SENSOR_INFO_ACTIVE_ARRAY_SIZE,
            Sensor::kResolution, 2);

    ADD_OR_SIZE(ANDROID_SENSOR_INFO_WHITE_LEVEL,
            &Sensor::kMaxRawValue, 1);

    static const int32_t blackLevelPattern[4] = {
        static_cast<int32_t>(Sensor::kBlackLevel),
        static_cast<int32_t>(Sensor::kBlackLevel),
        static_cast<int32_t>(Sensor::kBlackLevel),
        static_cast<int32_t>(Sensor::kBlackLevel)
    };
    ADD_OR_SIZE(ANDROID_SENSOR_BLACK_LEVEL_PATTERN,
            blackLevelPattern, sizeof(blackLevelPattern)/sizeof(int32_t));

    //TODO: sensor color calibration fields

    // android.flash
    static const uint8_t flashAvailable = 0;
    ADD_OR_SIZE(ANDROID_FLASH_INFO_AVAILABLE, &flashAvailable, 1);

    static const int64_t flashChargeDuration = 0;
    ADD_OR_SIZE(ANDROID_FLASH_INFO_CHARGE_DURATION, &flashChargeDuration, 1);

    // android.tonemap

    static const int32_t tonemapCurvePoints = 128;
    ADD_OR_SIZE(ANDROID_TONEMAP_MAX_CURVE_POINTS, &tonemapCurvePoints, 1);

    // android.scaler

    ADD_OR_SIZE(ANDROID_SCALER_AVAILABLE_FORMATS,
            kAvailableFormats,
            sizeof(kAvailableFormats)/sizeof(uint32_t));

    ADD_OR_SIZE(ANDROID_SCALER_AVAILABLE_RAW_SIZES,
            kAvailableRawSizes,
            sizeof(kAvailableRawSizes)/sizeof(uint32_t));

    ADD_OR_SIZE(ANDROID_SCALER_AVAILABLE_RAW_MIN_DURATIONS,
            kAvailableRawMinDurations,
            sizeof(kAvailableRawMinDurations)/sizeof(uint64_t));

    if (mFacingBack) {
        ADD_OR_SIZE(ANDROID_SCALER_AVAILABLE_PROCESSED_SIZES,
                kAvailableProcessedSizesBack,
                sizeof(kAvailableProcessedSizesBack)/sizeof(uint32_t));
    } else {
        ADD_OR_SIZE(ANDROID_SCALER_AVAILABLE_PROCESSED_SIZES,
                kAvailableProcessedSizesFront,
                sizeof(kAvailableProcessedSizesFront)/sizeof(uint32_t));
    }

    ADD_OR_SIZE(ANDROID_SCALER_AVAILABLE_PROCESSED_MIN_DURATIONS,
            kAvailableProcessedMinDurations,
            sizeof(kAvailableProcessedMinDurations)/sizeof(uint64_t));

    if (mFacingBack) {
        ADD_OR_SIZE(ANDROID_SCALER_AVAILABLE_JPEG_SIZES,
                kAvailableJpegSizesBack,
                sizeof(kAvailableJpegSizesBack)/sizeof(uint32_t));
    } else {
        ADD_OR_SIZE(ANDROID_SCALER_AVAILABLE_JPEG_SIZES,
                kAvailableJpegSizesFront,
                sizeof(kAvailableJpegSizesFront)/sizeof(uint32_t));
    }

    ADD_OR_SIZE(ANDROID_SCALER_AVAILABLE_JPEG_MIN_DURATIONS,
            kAvailableJpegMinDurations,
            sizeof(kAvailableJpegMinDurations)/sizeof(uint64_t));

    static const float maxZoom = 10;
    ADD_OR_SIZE(ANDROID_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM,
            &maxZoom, 1);

    // android.jpeg

    static const int32_t jpegThumbnailSizes[] = {
            0, 0,
            160, 120,
            320, 240
     };
    ADD_OR_SIZE(ANDROID_JPEG_AVAILABLE_THUMBNAIL_SIZES,
            jpegThumbnailSizes, sizeof(jpegThumbnailSizes)/sizeof(int32_t));

    static const int32_t jpegMaxSize = JpegCompressor::kMaxJpegSize;
    ADD_OR_SIZE(ANDROID_JPEG_MAX_SIZE, &jpegMaxSize, 1);

    // android.stats

    static const uint8_t availableFaceDetectModes[] = {
        ANDROID_STATISTICS_FACE_DETECT_MODE_OFF,
        ANDROID_STATISTICS_FACE_DETECT_MODE_SIMPLE,
        ANDROID_STATISTICS_FACE_DETECT_MODE_FULL
    };

    ADD_OR_SIZE(ANDROID_STATISTICS_INFO_AVAILABLE_FACE_DETECT_MODES,
            availableFaceDetectModes,
            sizeof(availableFaceDetectModes));

    static const int32_t maxFaceCount = 8;
    ADD_OR_SIZE(ANDROID_STATISTICS_INFO_MAX_FACE_COUNT,
            &maxFaceCount, 1);

    static const int32_t histogramSize = 64;
    ADD_OR_SIZE(ANDROID_STATISTICS_INFO_HISTOGRAM_BUCKET_COUNT,
            &histogramSize, 1);

    static const int32_t maxHistogramCount = 1000;
    ADD_OR_SIZE(ANDROID_STATISTICS_INFO_MAX_HISTOGRAM_COUNT,
            &maxHistogramCount, 1);

    static const int32_t sharpnessMapSize[2] = {64, 64};
    ADD_OR_SIZE(ANDROID_STATISTICS_INFO_SHARPNESS_MAP_SIZE,
            sharpnessMapSize, sizeof(sharpnessMapSize)/sizeof(int32_t));

    static const int32_t maxSharpnessMapValue = 1000;
    ADD_OR_SIZE(ANDROID_STATISTICS_INFO_MAX_SHARPNESS_MAP_VALUE,
            &maxSharpnessMapValue, 1);

    // android.control

    static const uint8_t availableSceneModes[] = {
            ANDROID_CONTROL_SCENE_MODE_DISABLED
    };
    ADD_OR_SIZE(ANDROID_CONTROL_AVAILABLE_SCENE_MODES,
            availableSceneModes, sizeof(availableSceneModes));

    static const uint8_t availableEffects[] = {
            ANDROID_CONTROL_EFFECT_MODE_OFF
    };
    ADD_OR_SIZE(ANDROID_CONTROL_AVAILABLE_EFFECTS,
            availableEffects, sizeof(availableEffects));

    static const int32_t max3aRegions[] = {/*AE*/ 0,/*AWB*/ 0,/*AF*/ 0};
    ADD_OR_SIZE(ANDROID_CONTROL_MAX_REGIONS,
            max3aRegions, sizeof(max3aRegions)/sizeof(max3aRegions[0]));

    static const uint8_t availableAeModes[] = {
            ANDROID_CONTROL_AE_MODE_OFF,
            ANDROID_CONTROL_AE_MODE_ON
    };
    ADD_OR_SIZE(ANDROID_CONTROL_AE_AVAILABLE_MODES,
            availableAeModes, sizeof(availableAeModes));

    static const camera_metadata_rational exposureCompensationStep = {
            1, 3
    };
    ADD_OR_SIZE(ANDROID_CONTROL_AE_COMPENSATION_STEP,
            &exposureCompensationStep, 1);

    int32_t exposureCompensationRange[] = {-9, 9};
    ADD_OR_SIZE(ANDROID_CONTROL_AE_COMPENSATION_RANGE,
            exposureCompensationRange,
            sizeof(exposureCompensationRange)/sizeof(int32_t));

    static const int32_t availableTargetFpsRanges[] = {
            5, 30, 15, 30
    };
    ADD_OR_SIZE(ANDROID_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES,
            availableTargetFpsRanges,
            sizeof(availableTargetFpsRanges)/sizeof(int32_t));

    static const uint8_t availableAntibandingModes[] = {
            ANDROID_CONTROL_AE_ANTIBANDING_MODE_OFF,
            ANDROID_CONTROL_AE_ANTIBANDING_MODE_AUTO
    };
    ADD_OR_SIZE(ANDROID_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES,
            availableAntibandingModes, sizeof(availableAntibandingModes));

    static const uint8_t availableAwbModes[] = {
            ANDROID_CONTROL_AWB_MODE_OFF,
            ANDROID_CONTROL_AWB_MODE_AUTO,
            ANDROID_CONTROL_AWB_MODE_INCANDESCENT,
            ANDROID_CONTROL_AWB_MODE_FLUORESCENT,
            ANDROID_CONTROL_AWB_MODE_DAYLIGHT,
            ANDROID_CONTROL_AWB_MODE_SHADE
    };
    ADD_OR_SIZE(ANDROID_CONTROL_AWB_AVAILABLE_MODES,
            availableAwbModes, sizeof(availableAwbModes));

    static const uint8_t availableAfModesBack[] = {
            ANDROID_CONTROL_AF_MODE_OFF,
            ANDROID_CONTROL_AF_MODE_AUTO,
            ANDROID_CONTROL_AF_MODE_MACRO,
            ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO,
            ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE
    };

    static const uint8_t availableAfModesFront[] = {
            ANDROID_CONTROL_AF_MODE_OFF
    };

    if (mFacingBack) {
        ADD_OR_SIZE(ANDROID_CONTROL_AF_AVAILABLE_MODES,
                    availableAfModesBack, sizeof(availableAfModesBack));
    } else {
        ADD_OR_SIZE(ANDROID_CONTROL_AF_AVAILABLE_MODES,
                    availableAfModesFront, sizeof(availableAfModesFront));
    }

    static const uint8_t availableVstabModes[] = {
            ANDROID_CONTROL_VIDEO_STABILIZATION_MODE_OFF
    };
    ADD_OR_SIZE(ANDROID_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES,
            availableVstabModes, sizeof(availableVstabModes));

#undef ADD_OR_SIZE
    /** Allocate metadata if sizing */
    if (sizeRequest) {
        ALOGV("Allocating %zu entries, %zu extra bytes for "
                "static camera info",
                entryCount, dataCount);
        *info = allocate_camera_metadata(entryCount, dataCount);
        if (*info == NULL) {
            ALOGE("Unable to allocate camera static info"
                    "(%zu entries, %zu bytes extra data)",
                    entryCount, dataCount);
            return NO_MEMORY;
        }
    }
    return OK;
}

status_t EmulatedFakeCamera2::constructDefaultRequest(
        int request_template,
        camera_metadata_t **request,
        bool sizeRequest) const {

    size_t entryCount = 0;
    size_t dataCount = 0;
    status_t ret;

#define ADD_OR_SIZE( tag, data, count ) \
    if ( ( ret = addOrSize(*request, sizeRequest, &entryCount, &dataCount, \
            tag, data, count) ) != OK ) return ret

    /** android.request */

    static const uint8_t requestType = ANDROID_REQUEST_TYPE_CAPTURE;
    ADD_OR_SIZE(ANDROID_REQUEST_TYPE, &requestType, 1);

    static const uint8_t metadataMode = ANDROID_REQUEST_METADATA_MODE_FULL;
    ADD_OR_SIZE(ANDROID_REQUEST_METADATA_MODE, &metadataMode, 1);

    static const int32_t id = 0;
    ADD_OR_SIZE(ANDROID_REQUEST_ID, &id, 1);

    static const int32_t frameCount = 0;
    ADD_OR_SIZE(ANDROID_REQUEST_FRAME_COUNT, &frameCount, 1);

    // OUTPUT_STREAMS set by user
    entryCount += 1;
    dataCount += 5; // TODO: Should be maximum stream number

    /** android.lens */

    static const float focusDistance = 0;
    ADD_OR_SIZE(ANDROID_LENS_FOCUS_DISTANCE, &focusDistance, 1);

    static const float aperture = 2.8f;
    ADD_OR_SIZE(ANDROID_LENS_APERTURE, &aperture, 1);

    static const float focalLength = 5.0f;
    ADD_OR_SIZE(ANDROID_LENS_FOCAL_LENGTH, &focalLength, 1);

    static const float filterDensity = 0;
    ADD_OR_SIZE(ANDROID_LENS_FILTER_DENSITY, &filterDensity, 1);

    static const uint8_t opticalStabilizationMode =
            ANDROID_LENS_OPTICAL_STABILIZATION_MODE_OFF;
    ADD_OR_SIZE(ANDROID_LENS_OPTICAL_STABILIZATION_MODE,
            &opticalStabilizationMode, 1);

    // FOCUS_RANGE set only in frame

    /** android.sensor */

    static const int64_t exposureTime = 10 * MSEC;
    ADD_OR_SIZE(ANDROID_SENSOR_EXPOSURE_TIME, &exposureTime, 1);

    static const int64_t frameDuration = 33333333L; // 1/30 s
    ADD_OR_SIZE(ANDROID_SENSOR_FRAME_DURATION, &frameDuration, 1);

    static const int32_t sensitivity = 100;
    ADD_OR_SIZE(ANDROID_SENSOR_SENSITIVITY, &sensitivity, 1);

    // TIMESTAMP set only in frame

    /** android.flash */

    static const uint8_t flashMode = ANDROID_FLASH_MODE_OFF;
    ADD_OR_SIZE(ANDROID_FLASH_MODE, &flashMode, 1);

    static const uint8_t flashPower = 10;
    ADD_OR_SIZE(ANDROID_FLASH_FIRING_POWER, &flashPower, 1);

    static const int64_t firingTime = 0;
    ADD_OR_SIZE(ANDROID_FLASH_FIRING_TIME, &firingTime, 1);

    /** Processing block modes */
    uint8_t hotPixelMode = 0;
    uint8_t demosaicMode = 0;
    uint8_t noiseMode = 0;
    uint8_t shadingMode = 0;
    uint8_t colorMode = 0;
    uint8_t tonemapMode = 0;
    uint8_t edgeMode = 0;
    switch (request_template) {
      case CAMERA2_TEMPLATE_STILL_CAPTURE:
        // fall-through
      case CAMERA2_TEMPLATE_VIDEO_SNAPSHOT:
        // fall-through
      case CAMERA2_TEMPLATE_ZERO_SHUTTER_LAG:
        hotPixelMode = ANDROID_HOT_PIXEL_MODE_HIGH_QUALITY;
        demosaicMode = ANDROID_DEMOSAIC_MODE_HIGH_QUALITY;
        noiseMode = ANDROID_NOISE_REDUCTION_MODE_HIGH_QUALITY;
        shadingMode = ANDROID_SHADING_MODE_HIGH_QUALITY;
        colorMode = ANDROID_COLOR_CORRECTION_MODE_HIGH_QUALITY;
        tonemapMode = ANDROID_TONEMAP_MODE_HIGH_QUALITY;
        edgeMode = ANDROID_EDGE_MODE_HIGH_QUALITY;
        break;
      case CAMERA2_TEMPLATE_PREVIEW:
        // fall-through
      case CAMERA2_TEMPLATE_VIDEO_RECORD:
        // fall-through
      default:
        hotPixelMode = ANDROID_HOT_PIXEL_MODE_FAST;
        demosaicMode = ANDROID_DEMOSAIC_MODE_FAST;
        noiseMode = ANDROID_NOISE_REDUCTION_MODE_FAST;
        shadingMode = ANDROID_SHADING_MODE_FAST;
        colorMode = ANDROID_COLOR_CORRECTION_MODE_FAST;
        tonemapMode = ANDROID_TONEMAP_MODE_FAST;
        edgeMode = ANDROID_EDGE_MODE_FAST;
        break;
    }
    ADD_OR_SIZE(ANDROID_HOT_PIXEL_MODE, &hotPixelMode, 1);
    ADD_OR_SIZE(ANDROID_DEMOSAIC_MODE, &demosaicMode, 1);
    ADD_OR_SIZE(ANDROID_NOISE_REDUCTION_MODE, &noiseMode, 1);
    ADD_OR_SIZE(ANDROID_SHADING_MODE, &shadingMode, 1);
    ADD_OR_SIZE(ANDROID_COLOR_CORRECTION_MODE, &colorMode, 1);
    ADD_OR_SIZE(ANDROID_TONEMAP_MODE, &tonemapMode, 1);
    ADD_OR_SIZE(ANDROID_EDGE_MODE, &edgeMode, 1);

    /** android.noise */
    static const uint8_t noiseStrength = 5;
    ADD_OR_SIZE(ANDROID_NOISE_REDUCTION_STRENGTH, &noiseStrength, 1);

    /** android.color */
    static const float colorTransform[9] = {
        1.0f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        0.f, 0.f, 1.f
    };
    ADD_OR_SIZE(ANDROID_COLOR_CORRECTION_TRANSFORM, colorTransform, 9);

    /** android.tonemap */
    static const float tonemapCurve[4] = {
        0.f, 0.f,
        1.f, 1.f
    };
    ADD_OR_SIZE(ANDROID_TONEMAP_CURVE_RED, tonemapCurve, 4);
    ADD_OR_SIZE(ANDROID_TONEMAP_CURVE_GREEN, tonemapCurve, 4);
    ADD_OR_SIZE(ANDROID_TONEMAP_CURVE_BLUE, tonemapCurve, 4);

    /** android.edge */
    static const uint8_t edgeStrength = 5;
    ADD_OR_SIZE(ANDROID_EDGE_STRENGTH, &edgeStrength, 1);

    /** android.scaler */
    static const int32_t cropRegion[3] = {
        0, 0, static_cast<int32_t>(Sensor::kResolution[0])
    };
    ADD_OR_SIZE(ANDROID_SCALER_CROP_REGION, cropRegion, 3);

    /** android.jpeg */
    static const int32_t jpegQuality = 80;
    ADD_OR_SIZE(ANDROID_JPEG_QUALITY, &jpegQuality, 1);

    static const int32_t thumbnailSize[2] = {
        640, 480
    };
    ADD_OR_SIZE(ANDROID_JPEG_THUMBNAIL_SIZE, thumbnailSize, 2);

    static const int32_t thumbnailQuality = 80;
    ADD_OR_SIZE(ANDROID_JPEG_THUMBNAIL_QUALITY, &thumbnailQuality, 1);

    static const double gpsCoordinates[2] = {
        0, 0
    };
    ADD_OR_SIZE(ANDROID_JPEG_GPS_COORDINATES, gpsCoordinates, 2);

    static const uint8_t gpsProcessingMethod[32] = "None";
    ADD_OR_SIZE(ANDROID_JPEG_GPS_PROCESSING_METHOD, gpsProcessingMethod, 32);

    static const int64_t gpsTimestamp = 0;
    ADD_OR_SIZE(ANDROID_JPEG_GPS_TIMESTAMP, &gpsTimestamp, 1);

    static const int32_t jpegOrientation = 0;
    ADD_OR_SIZE(ANDROID_JPEG_ORIENTATION, &jpegOrientation, 1);

    /** android.stats */

    static const uint8_t faceDetectMode =
        ANDROID_STATISTICS_FACE_DETECT_MODE_OFF;
    ADD_OR_SIZE(ANDROID_STATISTICS_FACE_DETECT_MODE, &faceDetectMode, 1);

    static const uint8_t histogramMode = ANDROID_STATISTICS_HISTOGRAM_MODE_OFF;
    ADD_OR_SIZE(ANDROID_STATISTICS_HISTOGRAM_MODE, &histogramMode, 1);

    static const uint8_t sharpnessMapMode =
        ANDROID_STATISTICS_SHARPNESS_MAP_MODE_OFF;
    ADD_OR_SIZE(ANDROID_STATISTICS_SHARPNESS_MAP_MODE, &sharpnessMapMode, 1);

    // faceRectangles, faceScores, faceLandmarks, faceIds, histogram,
    // sharpnessMap only in frames

    /** android.control */

    uint8_t controlIntent = 0;
    switch (request_template) {
      case CAMERA2_TEMPLATE_PREVIEW:
        controlIntent = ANDROID_CONTROL_CAPTURE_INTENT_PREVIEW;
        break;
      case CAMERA2_TEMPLATE_STILL_CAPTURE:
        controlIntent = ANDROID_CONTROL_CAPTURE_INTENT_STILL_CAPTURE;
        break;
      case CAMERA2_TEMPLATE_VIDEO_RECORD:
        controlIntent = ANDROID_CONTROL_CAPTURE_INTENT_VIDEO_RECORD;
        break;
      case CAMERA2_TEMPLATE_VIDEO_SNAPSHOT:
        controlIntent = ANDROID_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT;
        break;
      case CAMERA2_TEMPLATE_ZERO_SHUTTER_LAG:
        controlIntent = ANDROID_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG;
        break;
      default:
        controlIntent = ANDROID_CONTROL_CAPTURE_INTENT_CUSTOM;
        break;
    }
    ADD_OR_SIZE(ANDROID_CONTROL_CAPTURE_INTENT, &controlIntent, 1);

    static const uint8_t controlMode = ANDROID_CONTROL_MODE_AUTO;
    ADD_OR_SIZE(ANDROID_CONTROL_MODE, &controlMode, 1);

    static const uint8_t effectMode = ANDROID_CONTROL_EFFECT_MODE_OFF;
    ADD_OR_SIZE(ANDROID_CONTROL_EFFECT_MODE, &effectMode, 1);

    static const uint8_t sceneMode = ANDROID_CONTROL_SCENE_MODE_FACE_PRIORITY;
    ADD_OR_SIZE(ANDROID_CONTROL_SCENE_MODE, &sceneMode, 1);

    static const uint8_t aeMode = ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH;
    ADD_OR_SIZE(ANDROID_CONTROL_AE_MODE, &aeMode, 1);

    static const uint8_t aeLock = ANDROID_CONTROL_AE_LOCK_OFF;
    ADD_OR_SIZE(ANDROID_CONTROL_AE_LOCK, &aeLock, 1);

    static const int32_t controlRegions[5] = {
        0, 0,
        static_cast<int32_t>(Sensor::kResolution[0]),
        static_cast<int32_t>(Sensor::kResolution[1]),
        1000
    };
    ADD_OR_SIZE(ANDROID_CONTROL_AE_REGIONS, controlRegions, 5);

    static const int32_t aeExpCompensation = 0;
    ADD_OR_SIZE(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION, &aeExpCompensation, 1);

    static const int32_t aeTargetFpsRange[2] = {
        10, 30
    };
    ADD_OR_SIZE(ANDROID_CONTROL_AE_TARGET_FPS_RANGE, aeTargetFpsRange, 2);

    static const uint8_t aeAntibandingMode =
            ANDROID_CONTROL_AE_ANTIBANDING_MODE_AUTO;
    ADD_OR_SIZE(ANDROID_CONTROL_AE_ANTIBANDING_MODE, &aeAntibandingMode, 1);

    static const uint8_t awbMode =
            ANDROID_CONTROL_AWB_MODE_AUTO;
    ADD_OR_SIZE(ANDROID_CONTROL_AWB_MODE, &awbMode, 1);

    static const uint8_t awbLock = ANDROID_CONTROL_AWB_LOCK_OFF;
    ADD_OR_SIZE(ANDROID_CONTROL_AWB_LOCK, &awbLock, 1);

    ADD_OR_SIZE(ANDROID_CONTROL_AWB_REGIONS, controlRegions, 5);

    uint8_t afMode = 0;
    switch (request_template) {
      case CAMERA2_TEMPLATE_PREVIEW:
        afMode = ANDROID_CONTROL_AF_MODE_AUTO;
        break;
      case CAMERA2_TEMPLATE_STILL_CAPTURE:
        afMode = ANDROID_CONTROL_AF_MODE_AUTO;
        break;
      case CAMERA2_TEMPLATE_VIDEO_RECORD:
        afMode = ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO;
        break;
      case CAMERA2_TEMPLATE_VIDEO_SNAPSHOT:
        afMode = ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO;
        break;
      case CAMERA2_TEMPLATE_ZERO_SHUTTER_LAG:
        afMode = ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE;
        break;
      default:
        afMode = ANDROID_CONTROL_AF_MODE_AUTO;
        break;
    }
    ADD_OR_SIZE(ANDROID_CONTROL_AF_MODE, &afMode, 1);

    ADD_OR_SIZE(ANDROID_CONTROL_AF_REGIONS, controlRegions, 5);

    static const uint8_t vstabMode =
        ANDROID_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    ADD_OR_SIZE(ANDROID_CONTROL_VIDEO_STABILIZATION_MODE, &vstabMode, 1);

    // aeState, awbState, afState only in frame

    /** Allocate metadata if sizing */
    if (sizeRequest) {
        ALOGV("Allocating %zu entries, %zu extra bytes for "
                "request template type %d",
                entryCount, dataCount, request_template);
        *request = allocate_camera_metadata(entryCount, dataCount);
        if (*request == NULL) {
            ALOGE("Unable to allocate new request template type %d "
                    "(%zu entries, %zu bytes extra data)", request_template,
                    entryCount, dataCount);
            return NO_MEMORY;
        }
    }
    return OK;
#undef ADD_OR_SIZE
}

status_t EmulatedFakeCamera2::addOrSize(camera_metadata_t *request,
        bool sizeRequest,
        size_t *entryCount,
        size_t *dataCount,
        uint32_t tag,
        const void *entryData,
        size_t entryDataCount) {
    status_t res;
    if (!sizeRequest) {
        return add_camera_metadata_entry(request, tag, entryData,
                entryDataCount);
    } else {
        int type = get_camera_metadata_tag_type(tag);
        if (type < 0 ) return BAD_VALUE;
        (*entryCount)++;
        (*dataCount) += calculate_camera_metadata_entry_data_size(type,
                entryDataCount);
        return OK;
    }
}

bool EmulatedFakeCamera2::isStreamInUse(uint32_t id) {
    // Assumes mMutex is locked; otherwise new requests could enter
    // configureThread while readoutThread is being checked

    // Order of isStreamInUse calls matters
    if (mConfigureThread->isStreamInUse(id) ||
            mReadoutThread->isStreamInUse(id) ||
            mJpegCompressor->isStreamInUse(id) ) {
        ALOGE("%s: Stream %d is in use in active requests!",
                __FUNCTION__, id);
        return true;
    }
    return false;
}

bool EmulatedFakeCamera2::isReprocessStreamInUse(uint32_t id) {
    // TODO: implement
    return false;
}

const Stream& EmulatedFakeCamera2::getStreamInfo(uint32_t streamId) {
    Mutex::Autolock lock(mMutex);

    return mStreams.valueFor(streamId);
}

const ReprocessStream& EmulatedFakeCamera2::getReprocessStreamInfo(uint32_t streamId) {
    Mutex::Autolock lock(mMutex);

    return mReprocessStreams.valueFor(streamId);
}

};  /* namespace android */
