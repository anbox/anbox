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

#ifndef HW_EMULATOR_CAMERA_PREVIEW_WINDOW_H
#define HW_EMULATOR_CAMERA_PREVIEW_WINDOW_H

/*
 * Contains declaration of a class PreviewWindow that encapsulates functionality
 * of a preview window set via set_preview_window camera HAL API.
 */

namespace android {

class EmulatedCameraDevice;

/* Encapsulates functionality of a preview window set via set_preview_window
 * camera HAL API.
 *
 * Objects of this class are contained in EmulatedCamera objects, and handle
 * relevant camera API callbacks.
 */
class PreviewWindow {
public:
    /* Constructs PreviewWindow instance. */
    PreviewWindow();

    /* Destructs PreviewWindow instance. */
    ~PreviewWindow();

    /***************************************************************************
     * Camera API
     **************************************************************************/

public:
    /* Actual handler for camera_device_ops_t::set_preview_window callback.
     * This method is called by the containing emulated camera object when it is
     * handing the camera_device_ops_t::set_preview_window callback.
     * Param:
     *  window - Preview window to set. This parameter might be NULL, which
     *      indicates preview window reset.
     *  preview_fps - Preview's frame frequency. This parameter determins when
     *      a frame received via onNextFrameAvailable call will be pushed to
     *      the preview window. If 'window' parameter passed to this method is
     *      NULL, this parameter is ignored.
     * Return:
     *  NO_ERROR on success, or an appropriate error status.
     */
    status_t setPreviewWindow(struct preview_stream_ops* window,
                              int preview_fps);

    /* Starts the preview.
     * This method is called by the containing emulated camera object when it is
     * handing the camera_device_ops_t::start_preview callback.
     */
    status_t startPreview();

    /* Stops the preview.
     * This method is called by the containing emulated camera object when it is
     * handing the camera_device_ops_t::start_preview callback.
     */
    void stopPreview();

    /* Checks if preview is enabled. */
    inline bool isPreviewEnabled()
    {
        return mPreviewEnabled;
    }

    /****************************************************************************
     * Public API
     ***************************************************************************/

public:
    /* Next frame is available in the camera device.
     * This is a notification callback that is invoked by the camera device when
     * a new frame is available.
     * Note that most likely this method is called in context of a worker thread
     * that camera device has created for frame capturing.
     * Param:
     *  frame - Captured frame, or NULL if camera device didn't pull the frame
     *      yet. If NULL is passed in this parameter use GetCurrentFrame method
     *      of the camera device class to obtain the next frame. Also note that
     *      the size of the frame that is passed here (as well as the frame
     *      returned from the GetCurrentFrame method) is defined by the current
     *      frame settings (width + height + pixel format) for the camera device.
     * timestamp - Frame's timestamp.
     * camera_dev - Camera device instance that delivered the frame.
     */
    void onNextFrameAvailable(const void* frame,
                              nsecs_t timestamp,
                              EmulatedCameraDevice* camera_dev);

    /***************************************************************************
     * Private API
     **************************************************************************/

protected:
    /* Adjusts cached dimensions of the preview window frame according to the
     * frame dimensions used by the camera device.
     *
     * When preview is started, it's not known (hard to define) what are going
     * to be the dimensions of the frames that are going to be displayed. Plus,
     * it might be possible, that such dimensions can be changed on the fly. So,
     * in order to be always in sync with frame dimensions, this method is
     * called for each frame passed to onNextFrameAvailable method, in order to
     * properly adjust frame dimensions, used by the preview window.
     * Note that this method must be called while object is locked.
     * Param:
     *  camera_dev - Camera device, prpviding frames displayed in the preview
     *      window.
     * Return:
     *  true if cached dimensions have been adjusted, or false if cached
     *  dimensions match device's frame dimensions.
     */
    bool adjustPreviewDimensions(EmulatedCameraDevice* camera_dev);

    /* Checks if it's the time to push new frame to the preview window.
     * Note that this method must be called while object is locked. */
    bool isPreviewTime();

    /***************************************************************************
     * Data members
     **************************************************************************/

protected:
    /* Locks this instance for data changes. */
    Mutex                           mObjectLock;

    /* Preview window instance. */
    preview_stream_ops*             mPreviewWindow;

    /* Timestamp (abs. microseconds) when last frame has been pushed to the
     * preview window. */
    uint64_t                        mLastPreviewed;

    /* Preview frequency in microseconds. */
    uint32_t                        mPreviewAfter;

    /*
     * Cached preview window frame dimensions.
     */

    int                             mPreviewFrameWidth;
    int                             mPreviewFrameHeight;

    /* Preview status. */
    bool                            mPreviewEnabled;
};

}; /* namespace android */

#endif  /* HW_EMULATOR_CAMERA_PREVIEW_WINDOW_H */
