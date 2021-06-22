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
 * Adapted for Anbox by Alec Barber 2021
 */

#ifndef ANBOX_CAMERA_CAMERA_SERVICE_H_
#define ANBOX_CAMERA_CAMERA_SERVICE_H_

#include "anbox/do_not_copy_or_move.h"
#include "camera_common.h"

#include <string>

/*
 * Contains public camera service API.
 */

/* Initializes camera emulation service over qemu pipe. */
extern void android_camera_service_init(void);

/* Lists available web cameras. */
extern void android_list_web_cameras(void);

namespace anbox::camera {
    
/* Camera sevice descriptor. */
class CameraService : public anbox::DoNotCopyOrMove {
 private:
  CameraService();

  /* Initializes webcam emulation record in camera service descriptor.
   * Param:
   *  disp_name - Display name of a web camera ('webcam<N>') to use for emulation.
   *  dir - Direction ('back', or 'front') that emulated camera is facing.
   *  ci - Array of webcam information for enumerated web cameras connected
   *      to the host.
   */
  void setupWebcam(const std::string &disp_name,
                   const std::string &dir,
                   std::vector<CameraInfo> &ci);

  static const int MAX_CAMERA_DEFAULT = 8;

 public:

  /* Information about camera devices connected to the host.
   * Note that once initialized, entries in this array are considered to be
   * constant. */
  std::vector<CameraInfo>  camera_info;

  /* Gets camera information for the given camera device name.
   * Param:
   *  device_name - Camera's device name to look up the information for.
   * Return:
   *  Camera information pointer on success, or NULL if no camera information
   *  has been found for the given device name.
   */
  CameraInfo* getCameraInfoByDeviceName(const std::string &device_name);

  /* Get the single CameraService instance. */
  static std::shared_ptr<CameraService> get();
};
}

#endif  /* ANBOX_CAMERA_CAMERA_SERVICE_H_ */