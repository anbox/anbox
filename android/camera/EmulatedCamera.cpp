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
 * Contains implementation of a class EmulatedCamera that encapsulates
 * functionality common to all emulated cameras ("fake", "webcam", "video file",
 * etc.). Instances of this class (for each emulated camera) are created during
 * the construction of the EmulatedCameraFactory instance. This class serves as
 * an entry point for all camera API calls that defined by camera_device_ops_t
 * API.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera_Camera"
#include <cutils/log.h>
#include <ui/Rect.h>
#include "EmulatedCamera.h"
//#include "EmulatedFakeCameraDevice.h"
#include "Converters.h"

/* Defines whether we should trace parameter changes. */
#define DEBUG_PARAM 1

namespace android {

#if DEBUG_PARAM
/* Calculates and logs parameter changes.
 * Param:
 *  current - Current set of camera parameters.
 *  new_par - String representation of new parameters.
 */
static void PrintParamDiff(const CameraParameters& current, const char* new_par);
#else
#define PrintParamDiff(current, new_par)   (void(0))
#endif  /* DEBUG_PARAM */

/* A helper routine that adds a value to the camera parameter.
 * Param:
 *  param - Camera parameter to add a value to.
 *  val - Value to add.
 * Return:
 *  A new string containing parameter with the added value on success, or NULL on
 *  a failure. If non-NULL string is returned, the caller is responsible for
 *  freeing it with 'free'.
 */
static char* AddValue(const char* param, const char* val);

EmulatedCamera::EmulatedCamera(int cameraId,
                               struct hw_module_t* module)
        : EmulatedBaseCamera(cameraId,
                HARDWARE_DEVICE_API_VERSION(1, 0),
                &common,
                module),
          mPreviewWindow(),
          mCallbackNotifier()
{
    /* camera_device v1 fields. */
    common.close = EmulatedCamera::close;
    ops = &mDeviceOps;
    priv = this;
}

EmulatedCamera::~EmulatedCamera()
{
}

/****************************************************************************
 * Public API
 ***************************************************************************/

status_t EmulatedCamera::Initialize()
{
    /* Preview formats supported by this HAL. */
    char preview_formats[1024];
    snprintf(preview_formats, sizeof(preview_formats), "%s,%s,%s",
             CameraParameters::PIXEL_FORMAT_YUV420SP,
             CameraParameters::PIXEL_FORMAT_YUV420P,
             CameraParameters::PIXEL_FORMAT_RGBA8888);

    /*
     * Fake required parameters.
     */

    mParameters.set(CameraParameters::KEY_SUPPORTED_JPEG_THUMBNAIL_SIZES, "320x240,0x0");

    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_WIDTH, "512");
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_HEIGHT, "384");
    mParameters.set(CameraParameters::KEY_JPEG_QUALITY, "90");
    mParameters.set(CameraParameters::KEY_FOCAL_LENGTH, "4.31");
    mParameters.set(CameraParameters::KEY_HORIZONTAL_VIEW_ANGLE, "54.8");
    mParameters.set(CameraParameters::KEY_VERTICAL_VIEW_ANGLE, "42.5");
    mParameters.set(CameraParameters::KEY_JPEG_THUMBNAIL_QUALITY, "90");

    /* Preview format settings used here are related to panoramic view only. It's
     * not related to the preview window that works only with RGB frames, which
     * is explicitly stated when set_buffers_geometry is called on the preview
     * window object. */
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FORMATS,
                    preview_formats);
    mParameters.setPreviewFormat(CameraParameters::PIXEL_FORMAT_YUV420SP);

    /* We don't relay on the actual frame rates supported by the camera device,
     * since we will emulate them through timeouts in the emulated camera device
     * worker thread. */
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES,
                    "30,24,20,15,10,5");
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FPS_RANGE, "(5,30)");
    mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, "5,30");
    mParameters.setPreviewFrameRate(24);

    /* Only PIXEL_FORMAT_YUV420P is accepted by video framework in emulator! */
    mParameters.set(CameraParameters::KEY_VIDEO_FRAME_FORMAT,
                    CameraParameters::PIXEL_FORMAT_YUV420P);
    mParameters.set(CameraParameters::KEY_SUPPORTED_PICTURE_FORMATS,
                    CameraParameters::PIXEL_FORMAT_JPEG);
    mParameters.setPictureFormat(CameraParameters::PIXEL_FORMAT_JPEG);

    /* Set exposure compensation. */
    mParameters.set(CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION, "6");
    mParameters.set(CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION, "-6");
    mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP, "0.5");
    mParameters.set(CameraParameters::KEY_EXPOSURE_COMPENSATION, "0");

    /* Sets the white balance modes and the device-dependent scale factors. */
    char supported_white_balance[1024];
    snprintf(supported_white_balance, sizeof(supported_white_balance),
             "%s,%s,%s,%s",
             CameraParameters::WHITE_BALANCE_AUTO,
             CameraParameters::WHITE_BALANCE_INCANDESCENT,
             CameraParameters::WHITE_BALANCE_DAYLIGHT,
             CameraParameters::WHITE_BALANCE_TWILIGHT);
    mParameters.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE,
                    supported_white_balance);
    mParameters.set(CameraParameters::KEY_WHITE_BALANCE,
                    CameraParameters::WHITE_BALANCE_AUTO);
    getCameraDevice()->initializeWhiteBalanceModes(
            CameraParameters::WHITE_BALANCE_AUTO, 1.0f, 1.0f);
    getCameraDevice()->initializeWhiteBalanceModes(
            CameraParameters::WHITE_BALANCE_INCANDESCENT, 1.38f, 0.60f);
    getCameraDevice()->initializeWhiteBalanceModes(
            CameraParameters::WHITE_BALANCE_DAYLIGHT, 1.09f, 0.92f);
    getCameraDevice()->initializeWhiteBalanceModes(
            CameraParameters::WHITE_BALANCE_TWILIGHT, 0.92f, 1.22f);
    getCameraDevice()->setWhiteBalanceMode(CameraParameters::WHITE_BALANCE_AUTO);

    /* Not supported features
     */
    mParameters.set(CameraParameters::KEY_SUPPORTED_FOCUS_MODES,
                    CameraParameters::FOCUS_MODE_FIXED);
    mParameters.set(CameraParameters::KEY_FOCUS_MODE,
                    CameraParameters::FOCUS_MODE_FIXED);

    return NO_ERROR;
}

void EmulatedCamera::onNextFrameAvailable(const void* frame,
                                          nsecs_t timestamp,
                                          EmulatedCameraDevice* camera_dev)
{
    /* Notify the preview window first. */
    mPreviewWindow.onNextFrameAvailable(frame, timestamp, camera_dev);

    /* Notify callback notifier next. */
    mCallbackNotifier.onNextFrameAvailable(frame, timestamp, camera_dev);
}

void EmulatedCamera::onCameraDeviceError(int err)
{
    /* Errors are reported through the callback notifier */
    mCallbackNotifier.onCameraDeviceError(err);
}

/****************************************************************************
 * Camera API implementation.
 ***************************************************************************/

status_t EmulatedCamera::connectCamera(hw_device_t** device)
{
    ALOGV("%s", __FUNCTION__);

    status_t res = EINVAL;
    EmulatedCameraDevice* const camera_dev = getCameraDevice();
    ALOGE_IF(camera_dev == NULL, "%s: No camera device instance.", __FUNCTION__);

    if (camera_dev != NULL) {
        /* Connect to the camera device. */
        res = getCameraDevice()->connectDevice();
        if (res == NO_ERROR) {
            *device = &common;
        }
    }

    return -res;
}

status_t EmulatedCamera::closeCamera()
{
    ALOGV("%s", __FUNCTION__);

    return cleanupCamera();
}

status_t EmulatedCamera::getCameraInfo(struct camera_info* info)
{
    ALOGV("%s", __FUNCTION__);

    const char* valstr = NULL;

    valstr = mParameters.get(EmulatedCamera::FACING_KEY);
    if (valstr != NULL) {
        if (strcmp(valstr, EmulatedCamera::FACING_FRONT) == 0) {
            info->facing = CAMERA_FACING_FRONT;
        }
        else if (strcmp(valstr, EmulatedCamera::FACING_BACK) == 0) {
            info->facing = CAMERA_FACING_BACK;
        }
    } else {
        info->facing = CAMERA_FACING_BACK;
    }

    valstr = mParameters.get(EmulatedCamera::ORIENTATION_KEY);
    if (valstr != NULL) {
        info->orientation = atoi(valstr);
    } else {
        info->orientation = 0;
    }

    return EmulatedBaseCamera::getCameraInfo(info);
}

status_t EmulatedCamera::setPreviewWindow(struct preview_stream_ops* window)
{
    /* Callback should return a negative errno. */
    return -mPreviewWindow.setPreviewWindow(window,
                                             mParameters.getPreviewFrameRate());
}

void EmulatedCamera::setCallbacks(camera_notify_callback notify_cb,
                                  camera_data_callback data_cb,
                                  camera_data_timestamp_callback data_cb_timestamp,
                                  camera_request_memory get_memory,
                                  void* user)
{
    mCallbackNotifier.setCallbacks(notify_cb, data_cb, data_cb_timestamp,
                                    get_memory, user);
}

void EmulatedCamera::enableMsgType(int32_t msg_type)
{
    mCallbackNotifier.enableMessage(msg_type);
}

void EmulatedCamera::disableMsgType(int32_t msg_type)
{
    mCallbackNotifier.disableMessage(msg_type);
}

int EmulatedCamera::isMsgTypeEnabled(int32_t msg_type)
{
    return mCallbackNotifier.isMessageEnabled(msg_type);
}

status_t EmulatedCamera::startPreview()
{
    /* Callback should return a negative errno. */
    return -doStartPreview();
}

void EmulatedCamera::stopPreview()
{
    doStopPreview();
}

int EmulatedCamera::isPreviewEnabled()
{
    return mPreviewWindow.isPreviewEnabled();
}

status_t EmulatedCamera::storeMetaDataInBuffers(int enable)
{
    /* Callback should return a negative errno. */
    return -mCallbackNotifier.storeMetaDataInBuffers(enable);
}

status_t EmulatedCamera::startRecording()
{
    /* Callback should return a negative errno. */
    return -mCallbackNotifier.enableVideoRecording(mParameters.getPreviewFrameRate());
}

void EmulatedCamera::stopRecording()
{
    mCallbackNotifier.disableVideoRecording();
}

int EmulatedCamera::isRecordingEnabled()
{
    return mCallbackNotifier.isVideoRecordingEnabled();
}

void EmulatedCamera::releaseRecordingFrame(const void* opaque)
{
    mCallbackNotifier.releaseRecordingFrame(opaque);
}

status_t EmulatedCamera::setAutoFocus()
{
    ALOGV("%s", __FUNCTION__);

    /* TODO: Future enhancements. */
    return NO_ERROR;
}

status_t EmulatedCamera::cancelAutoFocus()
{
    ALOGV("%s", __FUNCTION__);

    /* TODO: Future enhancements. */
    return NO_ERROR;
}

status_t EmulatedCamera::takePicture()
{
    ALOGV("%s", __FUNCTION__);

    status_t res;
    int width, height;
    uint32_t org_fmt;

    /* Collect frame info for the picture. */
    mParameters.getPictureSize(&width, &height);
    const char* pix_fmt = mParameters.getPictureFormat();
    if (strcmp(pix_fmt, CameraParameters::PIXEL_FORMAT_YUV420P) == 0) {
        org_fmt = V4L2_PIX_FMT_YUV420;
    } else if (strcmp(pix_fmt, CameraParameters::PIXEL_FORMAT_RGBA8888) == 0) {
        org_fmt = V4L2_PIX_FMT_RGB32;
    } else if (strcmp(pix_fmt, CameraParameters::PIXEL_FORMAT_YUV420SP) == 0) {
        org_fmt = V4L2_PIX_FMT_NV21;
    } else if (strcmp(pix_fmt, CameraParameters::PIXEL_FORMAT_JPEG) == 0) {
        /* We only have JPEG converted for NV21 format. */
        org_fmt = V4L2_PIX_FMT_NV21;
    } else {
        ALOGE("%s: Unsupported pixel format %s", __FUNCTION__, pix_fmt);
        return EINVAL;
    }
    /* Get JPEG quality. */
    int jpeg_quality = mParameters.getInt(CameraParameters::KEY_JPEG_QUALITY);
    if (jpeg_quality <= 0) {
        jpeg_quality = 90;  /* Fall back to default. */
    }

    /*
     * Make sure preview is not running, and device is stopped before taking
     * picture.
     */

    const bool preview_on = mPreviewWindow.isPreviewEnabled();
    if (preview_on) {
        doStopPreview();
    }

    /* Camera device should have been stopped when the shutter message has been
     * enabled. */
    EmulatedCameraDevice* const camera_dev = getCameraDevice();
    if (camera_dev->isStarted()) {
        ALOGW("%s: Camera device is started", __FUNCTION__);
        camera_dev->stopDeliveringFrames();
        camera_dev->stopDevice();
    }

    /*
     * Take the picture now.
     */

    /* Start camera device for the picture frame. */
    ALOGD("Starting camera for picture: %.4s(%s)[%dx%d]",
         reinterpret_cast<const char*>(&org_fmt), pix_fmt, width, height);
    res = camera_dev->startDevice(width, height, org_fmt);
    if (res != NO_ERROR) {
        if (preview_on) {
            doStartPreview();
        }
        return res;
    }

    /* Deliver one frame only. */
    mCallbackNotifier.setJpegQuality(jpeg_quality);
    mCallbackNotifier.setTakingPicture(true);
    res = camera_dev->startDeliveringFrames(true);
    if (res != NO_ERROR) {
        mCallbackNotifier.setTakingPicture(false);
        if (preview_on) {
            doStartPreview();
        }
    }
    return res;
}

status_t EmulatedCamera::cancelPicture()
{
    ALOGV("%s", __FUNCTION__);

    return NO_ERROR;
}

status_t EmulatedCamera::setParameters(const char* parms)
{
    ALOGV("%s", __FUNCTION__);
    PrintParamDiff(mParameters, parms);

    CameraParameters new_param;
    String8 str8_param(parms);
    new_param.unflatten(str8_param);

    /*
     * Check for new exposure compensation parameter.
     */
    int new_exposure_compensation = new_param.getInt(
            CameraParameters::KEY_EXPOSURE_COMPENSATION);
    const int min_exposure_compensation = new_param.getInt(
            CameraParameters::KEY_MIN_EXPOSURE_COMPENSATION);
    const int max_exposure_compensation = new_param.getInt(
            CameraParameters::KEY_MAX_EXPOSURE_COMPENSATION);

    // Checks if the exposure compensation change is supported.
    if ((min_exposure_compensation != 0) || (max_exposure_compensation != 0)) {
        if (new_exposure_compensation > max_exposure_compensation) {
            new_exposure_compensation = max_exposure_compensation;
        }
        if (new_exposure_compensation < min_exposure_compensation) {
            new_exposure_compensation = min_exposure_compensation;
        }

        const int current_exposure_compensation = mParameters.getInt(
                CameraParameters::KEY_EXPOSURE_COMPENSATION);
        if (current_exposure_compensation != new_exposure_compensation) {
            const float exposure_value = new_exposure_compensation *
                    new_param.getFloat(
                            CameraParameters::KEY_EXPOSURE_COMPENSATION_STEP);

            getCameraDevice()->setExposureCompensation(
                    exposure_value);
        }
    }

    const char* new_white_balance = new_param.get(
            CameraParameters::KEY_WHITE_BALANCE);
    const char* supported_white_balance = new_param.get(
            CameraParameters::KEY_SUPPORTED_WHITE_BALANCE);

    if ((supported_white_balance != NULL) && (new_white_balance != NULL) &&
        (strstr(supported_white_balance, new_white_balance) != NULL)) {

        const char* current_white_balance = mParameters.get(
                CameraParameters::KEY_WHITE_BALANCE);
        if ((current_white_balance == NULL) ||
            (strcmp(current_white_balance, new_white_balance) != 0)) {
            ALOGV("Setting white balance to %s", new_white_balance);
            getCameraDevice()->setWhiteBalanceMode(new_white_balance);
        }
    }

    mParameters = new_param;

    return NO_ERROR;
}

/* A dumb variable indicating "no params" / error on the exit from
 * EmulatedCamera::getParameters(). */
static char lNoParam = '\0';
char* EmulatedCamera::getParameters()
{
    String8 params(mParameters.flatten());
    char* ret_str =
        reinterpret_cast<char*>(malloc(sizeof(char) * (params.length()+1)));
    memset(ret_str, 0, params.length()+1);
    if (ret_str != NULL) {
        strncpy(ret_str, params.string(), params.length()+1);
        return ret_str;
    } else {
        ALOGE("%s: Unable to allocate string for %s", __FUNCTION__, params.string());
        /* Apparently, we can't return NULL fron this routine. */
        return &lNoParam;
    }
}

void EmulatedCamera::putParameters(char* params)
{
    /* This method simply frees parameters allocated in getParameters(). */
    if (params != NULL && params != &lNoParam) {
        free(params);
    }
}

status_t EmulatedCamera::sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    ALOGV("%s: cmd = %d, arg1 = %d, arg2 = %d", __FUNCTION__, cmd, arg1, arg2);

    /* TODO: Future enhancements. */
    return 0;
}

void EmulatedCamera::releaseCamera()
{
    ALOGV("%s", __FUNCTION__);

    cleanupCamera();
}

status_t EmulatedCamera::dumpCamera(int fd)
{
    ALOGV("%s", __FUNCTION__);

    /* TODO: Future enhancements. */
    return -EINVAL;
}

/****************************************************************************
 * Preview management.
 ***************************************************************************/

status_t EmulatedCamera::doStartPreview()
{
    ALOGV("%s", __FUNCTION__);

    EmulatedCameraDevice* camera_dev = getCameraDevice();
    if (camera_dev->isStarted()) {
        camera_dev->stopDeliveringFrames();
        camera_dev->stopDevice();
    }

    status_t res = mPreviewWindow.startPreview();
    if (res != NO_ERROR) {
        return res;
    }

    /* Make sure camera device is connected. */
    if (!camera_dev->isConnected()) {
        res = camera_dev->connectDevice();
        if (res != NO_ERROR) {
            mPreviewWindow.stopPreview();
            return res;
        }
    }

    int width, height;
    /* Lets see what should we use for frame width, and height. */
    if (mParameters.get(CameraParameters::KEY_VIDEO_SIZE) != NULL) {
        mParameters.getVideoSize(&width, &height);
    } else {
        mParameters.getPreviewSize(&width, &height);
    }
    /* Lets see what should we use for the frame pixel format. Note that there
     * are two parameters that define pixel formats for frames sent to the
     * application via notification callbacks:
     * - KEY_VIDEO_FRAME_FORMAT, that is used when recording video, and
     * - KEY_PREVIEW_FORMAT, that is used for preview frame notification.
     * We choose one or the other, depending on "recording-hint" property set by
     * the framework that indicating intention: video, or preview. */
    const char* pix_fmt = NULL;
    const char* is_video = mParameters.get(EmulatedCamera::RECORDING_HINT_KEY);
    if (is_video == NULL) {
        is_video = CameraParameters::FALSE;
    }
    if (strcmp(is_video, CameraParameters::TRUE) == 0) {
        /* Video recording is requested. Lets see if video frame format is set. */
        pix_fmt = mParameters.get(CameraParameters::KEY_VIDEO_FRAME_FORMAT);
    }
    /* If this was not video recording, or video frame format is not set, lets
     * use preview pixel format for the main framebuffer. */
    if (pix_fmt == NULL) {
        pix_fmt = mParameters.getPreviewFormat();
    }
    if (pix_fmt == NULL) {
        ALOGE("%s: Unable to obtain video format", __FUNCTION__);
        mPreviewWindow.stopPreview();
        return EINVAL;
    }

    /* Convert framework's pixel format to the FOURCC one. */
    uint32_t org_fmt;
    if (strcmp(pix_fmt, CameraParameters::PIXEL_FORMAT_YUV420P) == 0) {
        org_fmt = V4L2_PIX_FMT_YUV420;
    } else if (strcmp(pix_fmt, CameraParameters::PIXEL_FORMAT_RGBA8888) == 0) {
        org_fmt = V4L2_PIX_FMT_RGB32;
    } else if (strcmp(pix_fmt, CameraParameters::PIXEL_FORMAT_YUV420SP) == 0) {
        org_fmt = V4L2_PIX_FMT_NV21;
    } else {
        ALOGE("%s: Unsupported pixel format %s", __FUNCTION__, pix_fmt);
        mPreviewWindow.stopPreview();
        return EINVAL;
    }
    ALOGD("Starting camera: %dx%d -> %.4s(%s)",
         width, height, reinterpret_cast<const char*>(&org_fmt), pix_fmt);
    res = camera_dev->startDevice(width, height, org_fmt);
    if (res != NO_ERROR) {
        mPreviewWindow.stopPreview();
        return res;
    }

    res = camera_dev->startDeliveringFrames(false);
    if (res != NO_ERROR) {
        camera_dev->stopDevice();
        mPreviewWindow.stopPreview();
    }

    return res;
}

status_t EmulatedCamera::doStopPreview()
{
    ALOGV("%s", __FUNCTION__);

    status_t res = NO_ERROR;
    if (mPreviewWindow.isPreviewEnabled()) {
        /* Stop the camera. */
        if (getCameraDevice()->isStarted()) {
            getCameraDevice()->stopDeliveringFrames();
            res = getCameraDevice()->stopDevice();
        }

        if (res == NO_ERROR) {
            /* Disable preview as well. */
            mPreviewWindow.stopPreview();
        }
    }

    return NO_ERROR;
}

/****************************************************************************
 * Private API.
 ***************************************************************************/

status_t EmulatedCamera::cleanupCamera()
{
    status_t res = NO_ERROR;

    /* If preview is running - stop it. */
    res = doStopPreview();
    if (res != NO_ERROR) {
        return -res;
    }

    /* Stop and disconnect the camera device. */
    EmulatedCameraDevice* const camera_dev = getCameraDevice();
    if (camera_dev != NULL) {
        if (camera_dev->isStarted()) {
            camera_dev->stopDeliveringFrames();
            res = camera_dev->stopDevice();
            if (res != NO_ERROR) {
                return -res;
            }
        }
        if (camera_dev->isConnected()) {
            res = camera_dev->disconnectDevice();
            if (res != NO_ERROR) {
                return -res;
            }
        }
    }

    mCallbackNotifier.cleanupCBNotifier();

    return NO_ERROR;
}

/****************************************************************************
 * Camera API callbacks as defined by camera_device_ops structure.
 *
 * Callbacks here simply dispatch the calls to an appropriate method inside
 * EmulatedCamera instance, defined by the 'dev' parameter.
 ***************************************************************************/

int EmulatedCamera::set_preview_window(struct camera_device* dev,
                                       struct preview_stream_ops* window)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->setPreviewWindow(window);
}

void EmulatedCamera::set_callbacks(
        struct camera_device* dev,
        camera_notify_callback notify_cb,
        camera_data_callback data_cb,
        camera_data_timestamp_callback data_cb_timestamp,
        camera_request_memory get_memory,
        void* user)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->setCallbacks(notify_cb, data_cb, data_cb_timestamp, get_memory, user);
}

void EmulatedCamera::enable_msg_type(struct camera_device* dev, int32_t msg_type)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->enableMsgType(msg_type);
}

void EmulatedCamera::disable_msg_type(struct camera_device* dev, int32_t msg_type)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->disableMsgType(msg_type);
}

int EmulatedCamera::msg_type_enabled(struct camera_device* dev, int32_t msg_type)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->isMsgTypeEnabled(msg_type);
}

int EmulatedCamera::start_preview(struct camera_device* dev)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->startPreview();
}

void EmulatedCamera::stop_preview(struct camera_device* dev)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->stopPreview();
}

int EmulatedCamera::preview_enabled(struct camera_device* dev)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->isPreviewEnabled();
}

int EmulatedCamera::store_meta_data_in_buffers(struct camera_device* dev,
                                               int enable)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->storeMetaDataInBuffers(enable);
}

int EmulatedCamera::start_recording(struct camera_device* dev)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->startRecording();
}

void EmulatedCamera::stop_recording(struct camera_device* dev)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->stopRecording();
}

int EmulatedCamera::recording_enabled(struct camera_device* dev)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->isRecordingEnabled();
}

void EmulatedCamera::release_recording_frame(struct camera_device* dev,
                                             const void* opaque)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->releaseRecordingFrame(opaque);
}

int EmulatedCamera::auto_focus(struct camera_device* dev)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->setAutoFocus();
}

int EmulatedCamera::cancel_auto_focus(struct camera_device* dev)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->cancelAutoFocus();
}

int EmulatedCamera::take_picture(struct camera_device* dev)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->takePicture();
}

int EmulatedCamera::cancel_picture(struct camera_device* dev)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->cancelPicture();
}

int EmulatedCamera::set_parameters(struct camera_device* dev, const char* parms)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->setParameters(parms);
}

char* EmulatedCamera::get_parameters(struct camera_device* dev)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return NULL;
    }
    return ec->getParameters();
}

void EmulatedCamera::put_parameters(struct camera_device* dev, char* params)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->putParameters(params);
}

int EmulatedCamera::send_command(struct camera_device* dev,
                                 int32_t cmd,
                                 int32_t arg1,
                                 int32_t arg2)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->sendCommand(cmd, arg1, arg2);
}

void EmulatedCamera::release(struct camera_device* dev)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return;
    }
    ec->releaseCamera();
}

int EmulatedCamera::dump(struct camera_device* dev, int fd)
{
    EmulatedCamera* ec = reinterpret_cast<EmulatedCamera*>(dev->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->dumpCamera(fd);
}

int EmulatedCamera::close(struct hw_device_t* device)
{
    EmulatedCamera* ec =
        reinterpret_cast<EmulatedCamera*>(reinterpret_cast<struct camera_device*>(device)->priv);
    if (ec == NULL) {
        ALOGE("%s: Unexpected NULL camera device", __FUNCTION__);
        return -EINVAL;
    }
    return ec->closeCamera();
}

/****************************************************************************
 * Static initializer for the camera callback API
 ****************************************************************************/

camera_device_ops_t EmulatedCamera::mDeviceOps = {
    EmulatedCamera::set_preview_window,
    EmulatedCamera::set_callbacks,
    EmulatedCamera::enable_msg_type,
    EmulatedCamera::disable_msg_type,
    EmulatedCamera::msg_type_enabled,
    EmulatedCamera::start_preview,
    EmulatedCamera::stop_preview,
    EmulatedCamera::preview_enabled,
    EmulatedCamera::store_meta_data_in_buffers,
    EmulatedCamera::start_recording,
    EmulatedCamera::stop_recording,
    EmulatedCamera::recording_enabled,
    EmulatedCamera::release_recording_frame,
    EmulatedCamera::auto_focus,
    EmulatedCamera::cancel_auto_focus,
    EmulatedCamera::take_picture,
    EmulatedCamera::cancel_picture,
    EmulatedCamera::set_parameters,
    EmulatedCamera::get_parameters,
    EmulatedCamera::put_parameters,
    EmulatedCamera::send_command,
    EmulatedCamera::release,
    EmulatedCamera::dump
};

/****************************************************************************
 * Common keys
 ***************************************************************************/

const char EmulatedCamera::FACING_KEY[]         = "prop-facing";
const char EmulatedCamera::ORIENTATION_KEY[]    = "prop-orientation";
const char EmulatedCamera::RECORDING_HINT_KEY[] = "recording-hint";

/****************************************************************************
 * Common string values
 ***************************************************************************/

const char EmulatedCamera::FACING_BACK[]      = "back";
const char EmulatedCamera::FACING_FRONT[]     = "front";

/****************************************************************************
 * Helper routines
 ***************************************************************************/

static char* AddValue(const char* param, const char* val)
{
    const size_t len1 = strlen(param);
    const size_t len2 = strlen(val);
    char* ret = reinterpret_cast<char*>(malloc(len1 + len2 + 2));
    ALOGE_IF(ret == NULL, "%s: Memory failure", __FUNCTION__);
    if (ret != NULL) {
        memcpy(ret, param, len1);
        ret[len1] = ',';
        memcpy(ret + len1 + 1, val, len2);
        ret[len1 + len2 + 1] = '\0';
    }
    return ret;
}

/****************************************************************************
 * Parameter debugging helpers
 ***************************************************************************/

#if DEBUG_PARAM
static void PrintParamDiff(const CameraParameters& current,
                            const char* new_par)
{
    char tmp[2048];
    const char* wrk = new_par;

    /* Divided with ';' */
    const char* next = strchr(wrk, ';');
    while (next != NULL) {
        snprintf(tmp, sizeof(tmp), "%.*s", (int)(intptr_t)(next-wrk), wrk);
        /* in the form key=value */
        char* val = strchr(tmp, '=');
        if (val != NULL) {
            *val = '\0'; val++;
            const char* in_current = current.get(tmp);
            if (in_current != NULL) {
                if (strcmp(in_current, val)) {
                    ALOGD("=== Value changed: %s: %s -> %s", tmp, in_current, val);
                }
            } else {
                ALOGD("+++ New parameter: %s=%s", tmp, val);
            }
        } else {
            ALOGW("No value separator in %s", tmp);
        }
        wrk = next + 1;
        next = strchr(wrk, ';');
    }
}
#endif  /* DEBUG_PARAM */

}; /* namespace android */
