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
 * 
 * Adapted for Anbox by Alec Barber 2021.
 */

#ifndef ANBOX_CAMERA_CAMERA_DEVICE_H
#define ANBOX_CAMERA_CAMERA_DEVICE_H

#include "camera_common.h"
#include "anbox/do_not_copy_or_move.h"
#include <string>
#include <memory>

namespace anbox::camera {

/* Representation of a camera device which communicates with the emulator
 */
class CameraDevice : public DoNotCopyOrMove {
 public:
  virtual ~CameraDevice();

  /* Initializes camera device descriptor, and connects to the camera device.
   * Param:
   *  name - On Linux contains name of the device to be used to capture video.
   *    On Windows contains name to assign to the capturing window. This
   *    parameter can be NULL, in which case '/dev/video0' will be used as
   *    device name on
   *    Linux, or 'AndroidEmulatorVC' on Windows.
   *  inp_channel - On Linux defines input channel to use when communicating with
   *    the camera driver. On Windows contains an index (up to 10) of the driver
   *    to use to communicate with the camera device.
   * Return:
   *  Initialized camera device descriptor on success, or NULL on failure.
   */
  static std::shared_ptr<CameraDevice>
  openCameraDevice(const char *name, int inp_channel);

  /* Starts capturing frames from the camera device. The camera device must be
   * in the open state.
   * Param:
   *  pixel_format - Defines pixel format for the captured frames. Must be one of
   *      the formats, supported by the camera device.
   *  width, height - Frame dimensions for the captured video frame. Must match
   *      dimensions supported by the camera for the pixel format defined by the
   *      'pixel_format' parameter.
   * Return:
   *  0 on success, or non-zero value on failure.
   */
  virtual int startCapturing(uint32_t pixel_format,
                             unsigned int frame_width,
                             unsigned int frame_height) = 0;
  
  /* Stops capturing frames from the camera device.
   * Return:
   *  0 on success, or non-zero value on failure.
   */
  virtual int stopCapturing() = 0;

  /* Captures a frame from the camera device.
   * Param:
   *  framebuffers - Array of framebuffers where to read the frame. Size of this
   *      array is defined by the 'fbs_num' parameter. Note that the caller must
   *      make sure that buffers are large enough to contain entire frame
   *      captured from the device.
   *  fbs_num - Number of entries in the 'framebuffers' array.
   *  r_scale, g_scale, b_scale - White balance scale.
   *  exp_comp - Exposure compensation.
   * Return:
   *  0 on success, or non-zero value on failure. There is a special vaule 1
   *  returned from this routine which indicates that frames were not available in
   *  the device. This value is returned on Linux implementation when frame ioctl
   *  has returned EAGAIN error. The client should respond to this value by
   *  repeating the read, rather than reporting an error.
   */
  virtual int readFrame(ClientFrameBuffer* framebuffers,
                        int fbs_num,
                        float r_scale,
                        float g_scale,
                        float b_scale,
                        float exp_comp) = 0;
  

  /* Closes open camera device.
   */
  virtual void closeCameraDevice() = 0;

  /* Enumerates camera devices connected to the host, and collects information
   * about each device.
   * Apparently, camera framework in the guest will only accept the the YV12
   * (V4L2_PIX_FMT_YVU420) pixel format. So, we don't really need to report all the
   * pixel formats, supported by the camera device back to the guest. We can simply
   * pick any format that is supported by the device, and collect frame dimensions
   * available for it. The only thing we can do is to specifically check, if camera
   * support YV12, and choose it, in order to spare some CPU cycles on the
   * conversion.
   * Param:
   *  max - Maximum number of entries that can fit into the array.
   * Return:
   *  A vector of CameraInfos on success, or NULL on failure.
   */
  static std::shared_ptr<std::vector<CameraInfo>> enumerateCameraDevices(int max);
};

}

#endif  /* ANBOX_CAMERA_CAMERA_DEVICE_H */
