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
 * Contains implementation of a class CallbackNotifier that manages callbacks set
 * via set_callbacks, enable_msg_type, and disable_msg_type camera HAL API.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera_CallbackNotifier"
#include <cutils/log.h>
#include <MetadataBufferType.h>
#include "EmulatedCameraDevice.h"
#include "CallbackNotifier.h"
#include "JpegCompressor.h"

namespace android {

/* String representation of camera messages. */
static const char* lCameraMessages[] =
{
    "CAMERA_MSG_ERROR",
    "CAMERA_MSG_SHUTTER",
    "CAMERA_MSG_FOCUS",
    "CAMERA_MSG_ZOOM",
    "CAMERA_MSG_PREVIEW_FRAME",
    "CAMERA_MSG_VIDEO_FRAME",
    "CAMERA_MSG_POSTVIEW_FRAME",
    "CAMERA_MSG_RAW_IMAGE",
    "CAMERA_MSG_COMPRESSED_IMAGE",
    "CAMERA_MSG_RAW_IMAGE_NOTIFY",
    "CAMERA_MSG_PREVIEW_METADATA"
};
static const int lCameraMessagesNum = sizeof(lCameraMessages) / sizeof(char*);

/* Builds an array of strings for the given set of messages.
 * Param:
 *  msg - Messages to get strings for,
 *  strings - Array where to save strings
 *  max - Maximum number of entries in the array.
 * Return:
 *  Number of strings saved into the 'strings' array.
 */
static int GetMessageStrings(uint32_t msg, const char** strings, int max)
{
    int index = 0;
    int out = 0;
    while (msg != 0 && out < max && index < lCameraMessagesNum) {
        while ((msg & 0x1) == 0 && index < lCameraMessagesNum) {
            msg >>= 1;
            index++;
        }
        if ((msg & 0x1) != 0 && index < lCameraMessagesNum) {
            strings[out] = lCameraMessages[index];
            out++;
            msg >>= 1;
            index++;
        }
    }

    return out;
}

/* Logs messages, enabled by the mask. */
static void PrintMessages(uint32_t msg)
{
    const char* strs[lCameraMessagesNum];
    const int translated = GetMessageStrings(msg, strs, lCameraMessagesNum);
    for (int n = 0; n < translated; n++) {
        ALOGV("    %s", strs[n]);
    }
}

CallbackNotifier::CallbackNotifier()
    : mNotifyCB(NULL),
      mDataCB(NULL),
      mDataCBTimestamp(NULL),
      mGetMemoryCB(NULL),
      mCBOpaque(NULL),
      mLastFrameTimestamp(0),
      mFrameRefreshFreq(0),
      mMessageEnabler(0),
      mJpegQuality(90),
      mVideoRecEnabled(false),
      mTakingPicture(false)
{
}

CallbackNotifier::~CallbackNotifier()
{
}

/****************************************************************************
 * Camera API
 ***************************************************************************/

void CallbackNotifier::setCallbacks(camera_notify_callback notify_cb,
                                    camera_data_callback data_cb,
                                    camera_data_timestamp_callback data_cb_timestamp,
                                    camera_request_memory get_memory,
                                    void* user)
{
    ALOGV("%s: %p, %p, %p, %p (%p)",
         __FUNCTION__, notify_cb, data_cb, data_cb_timestamp, get_memory, user);

    Mutex::Autolock locker(&mObjectLock);
    mNotifyCB = notify_cb;
    mDataCB = data_cb;
    mDataCBTimestamp = data_cb_timestamp;
    mGetMemoryCB = get_memory;
    mCBOpaque = user;
}

void CallbackNotifier::enableMessage(uint msg_type)
{
    ALOGV("%s: msg_type = 0x%x", __FUNCTION__, msg_type);
    PrintMessages(msg_type);

    Mutex::Autolock locker(&mObjectLock);
    mMessageEnabler |= msg_type;
    ALOGV("**** Currently enabled messages:");
    PrintMessages(mMessageEnabler);
}

void CallbackNotifier::disableMessage(uint msg_type)
{
    ALOGV("%s: msg_type = 0x%x", __FUNCTION__, msg_type);
    PrintMessages(msg_type);

    Mutex::Autolock locker(&mObjectLock);
    mMessageEnabler &= ~msg_type;
    ALOGV("**** Currently enabled messages:");
    PrintMessages(mMessageEnabler);
}

status_t CallbackNotifier::enableVideoRecording(int fps)
{
    ALOGV("%s: FPS = %d", __FUNCTION__, fps);

    Mutex::Autolock locker(&mObjectLock);
    mVideoRecEnabled = true;
    mLastFrameTimestamp = 0;
    mFrameRefreshFreq = 1000000000LL / fps;

    return NO_ERROR;
}

void CallbackNotifier::disableVideoRecording()
{
    ALOGV("%s:", __FUNCTION__);

    Mutex::Autolock locker(&mObjectLock);
    mVideoRecEnabled = false;
    mLastFrameTimestamp = 0;
    mFrameRefreshFreq = 0;
}

void CallbackNotifier::releaseRecordingFrame(const void* opaque)
{
    List<camera_memory_t*>::iterator it = mCameraMemoryTs.begin();
    for( ; it != mCameraMemoryTs.end(); ++it ) {
        if ( (*it)->data == opaque ) {
            (*it)->release( *it );
            mCameraMemoryTs.erase(it);
            break;
        }
    }
}

status_t CallbackNotifier::storeMetaDataInBuffers(bool enable)
{
    // Return error if metadata is request, otherwise silently agree.
    return enable ? INVALID_OPERATION : NO_ERROR;
}

/****************************************************************************
 * Public API
 ***************************************************************************/

void CallbackNotifier::cleanupCBNotifier()
{
    Mutex::Autolock locker(&mObjectLock);
    mMessageEnabler = 0;
    mNotifyCB = NULL;
    mDataCB = NULL;
    mDataCBTimestamp = NULL;
    mGetMemoryCB = NULL;
    mCBOpaque = NULL;
    mLastFrameTimestamp = 0;
    mFrameRefreshFreq = 0;
    mJpegQuality = 90;
    mVideoRecEnabled = false;
    mTakingPicture = false;
}

void CallbackNotifier::onNextFrameAvailable(const void* frame,
                                            nsecs_t timestamp,
                                            EmulatedCameraDevice* camera_dev)
{
    if (isMessageEnabled(CAMERA_MSG_VIDEO_FRAME) && isVideoRecordingEnabled() &&
            isNewVideoFrameTime(timestamp)) {
        camera_memory_t* cam_buff =
            mGetMemoryCB(-1, camera_dev->getFrameBufferSize(), 1, NULL);
        if (NULL != cam_buff && NULL != cam_buff->data) {
            memcpy(cam_buff->data, frame, camera_dev->getFrameBufferSize());
            mDataCBTimestamp(timestamp, CAMERA_MSG_VIDEO_FRAME,
                               cam_buff, 0, mCBOpaque);

            mCameraMemoryTs.push_back( cam_buff );
        } else {
            ALOGE("%s: Memory failure in CAMERA_MSG_VIDEO_FRAME", __FUNCTION__);
        }
    }

    if (isMessageEnabled(CAMERA_MSG_PREVIEW_FRAME)) {
        camera_memory_t* cam_buff =
            mGetMemoryCB(-1, camera_dev->getFrameBufferSize(), 1, NULL);
        if (NULL != cam_buff && NULL != cam_buff->data) {
            memcpy(cam_buff->data, frame, camera_dev->getFrameBufferSize());
            mDataCB(CAMERA_MSG_PREVIEW_FRAME, cam_buff, 0, NULL, mCBOpaque);
            cam_buff->release(cam_buff);
        } else {
            ALOGE("%s: Memory failure in CAMERA_MSG_PREVIEW_FRAME", __FUNCTION__);
        }
    }

    if (mTakingPicture) {
        /* This happens just once. */
        mTakingPicture = false;
        /* The sequence of callbacks during picture taking is:
         *  - CAMERA_MSG_SHUTTER
         *  - CAMERA_MSG_RAW_IMAGE_NOTIFY
         *  - CAMERA_MSG_COMPRESSED_IMAGE
         */
        if (isMessageEnabled(CAMERA_MSG_SHUTTER)) {
            mNotifyCB(CAMERA_MSG_SHUTTER, 0, 0, mCBOpaque);
        }
        if (isMessageEnabled(CAMERA_MSG_RAW_IMAGE_NOTIFY)) {
            mNotifyCB(CAMERA_MSG_RAW_IMAGE_NOTIFY, 0, 0, mCBOpaque);
        }
        if (isMessageEnabled(CAMERA_MSG_COMPRESSED_IMAGE)) {
            /* Compress the frame to JPEG. Note that when taking pictures, we
             * have requested camera device to provide us with NV21 frames. */
            NV21JpegCompressor compressor;
            status_t res =
                compressor.compressRawImage(frame, camera_dev->getFrameWidth(),
                                            camera_dev->getFrameHeight(),
                                            mJpegQuality);
            if (res == NO_ERROR) {
                camera_memory_t* jpeg_buff =
                    mGetMemoryCB(-1, compressor.getCompressedSize(), 1, NULL);
                if (NULL != jpeg_buff && NULL != jpeg_buff->data) {
                    compressor.getCompressedImage(jpeg_buff->data);
                    mDataCB(CAMERA_MSG_COMPRESSED_IMAGE, jpeg_buff, 0, NULL, mCBOpaque);
                    jpeg_buff->release(jpeg_buff);
                } else {
                    ALOGE("%s: Memory failure in CAMERA_MSG_VIDEO_FRAME", __FUNCTION__);
                }
            } else {
                ALOGE("%s: Compression failure in CAMERA_MSG_VIDEO_FRAME", __FUNCTION__);
            }
        }
    }
}

void CallbackNotifier::onCameraDeviceError(int err)
{
    if (isMessageEnabled(CAMERA_MSG_ERROR) && mNotifyCB != NULL) {
        mNotifyCB(CAMERA_MSG_ERROR, err, 0, mCBOpaque);
    }
}

/****************************************************************************
 * Private API
 ***************************************************************************/

bool CallbackNotifier::isNewVideoFrameTime(nsecs_t timestamp)
{
    Mutex::Autolock locker(&mObjectLock);
    if ((timestamp - mLastFrameTimestamp) >= mFrameRefreshFreq) {
        mLastFrameTimestamp = timestamp;
        return true;
    }
    return false;
}

}; /* namespace android */
