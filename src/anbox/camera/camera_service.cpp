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
 * Contains emulated camera service implementation.
 */

#include "anbox/camera/camera_device.h"
#include "anbox/camera/camera_format_converters.h"
#include "anbox/camera/camera_service.h"
#include "anbox/do_not_copy_or_move.h"
#include "anbox/logger.h"
#include "anbox/utils.h"

#define  E(...)    ERROR(__VA_ARGS__)
#define  W(...)    WARNING(__VA_ARGS__)
#define  D(...)    DEBUG(__VA_ARGS__)
#define  D_ACTIVE  1

/*******************************************************************************
 * CameraService API
 ******************************************************************************/

namespace anbox::camera {

/* Initializes camera service descriptor.
 */
CameraService::CameraService() {
  std::string hw_camera_back = utils::get_env_value("ANBOX_HW_CAMERA_BACK", "");
  std::string hw_camera_front = utils::get_env_value("ANBOX_HW_CAMERA_FRONT", "");

  /* Lets see if HW config uses web cameras. */
  if (!utils::string_starts_with(hw_camera_back, "webcam") &&
      !utils::string_starts_with(hw_camera_front, "webcam")) {
    /* Web camera emulation is disabled. Skip enumeration of webcameras. */
    return;
  }

  /* Enumerate web cameras connected to the host. */
  int max_camera = atoi(utils::get_env_value("ANBOX_HW_CAMERA_MAX_CAMERA",
      std::to_string(MAX_CAMERA_DEFAULT)).c_str());
  const auto ci = anbox::camera::CameraDevice::enumerateCameraDevices(max_camera);
  if (!ci) {
    /* An error occurred */
    E("Error while enumerating camera devices");
    return;
  }
  D("Found %d hardware cameras", ci->size());
  if (ci->size() == 0) {
    /* Nothing is connected - nothing to emulate. */
    return;
  }

  /* Set up back camera emulation. */
  if (utils::string_starts_with(hw_camera_back, "webcam")) {
    setupWebcam(hw_camera_back.c_str(), "back", *ci);
  }

  /* Set up front camera emulation. */
  if (utils::string_starts_with(hw_camera_front, "webcam")) {
    setupWebcam(hw_camera_front.c_str(), "front", *ci);
  }
}

/* Initializes webcam emulation record in camera service descriptor.
 * Param:
 *  disp_name - Display name of a web camera ('webcam<N>') to use for emulation.
 *  dir - Direction ('back', or 'front') that emulated camera is facing.
 *  ci - Array of webcam information for enumerated web cameras connected
 *      to the host.
 */
void CameraService::setupWebcam(const std::string &disp_name,
                                const std::string &dir,
                                std::vector<CameraInfo> &ci) {
  /* Find webcam record in the list of enumerated web cameras. */
  CameraInfo* found = NULL;
  for (size_t n = 0; n < ci.size(); n++) {
    if (!ci[n].in_use && ci[n].display_name == disp_name) {
      found = &ci[n];
      break;
    }
  }
  if (found == NULL) {
    W("Camera name '%s' is not found in the list of connected cameras.\n",
      disp_name);
    return;
  }

  /* Save to the camera info array that will be used by the service. */
  camera_info.push_back(*found);
  /* This camera is taken. */
  found->in_use = 1;
  camera_info.back().direction = dir;
  DEBUG("Camera %d '%s' connected to '%s' facing %s using %.4s pixel format",
    camera_info.size() - 1, camera_info.back().display_name,
    camera_info.back().device_name,
    camera_info.back().direction,
    reinterpret_cast<const char*>(&camera_info.back().pixel_format));
}

/* Gets camera information for the given camera device name.
 *  device_name - Camera's device name to look up the information for.
 * Return:
 *  Camera information pointer on success, or NULL if no camera information has
 *  been found for the given device name.
 */
CameraInfo* CameraService::getCameraInfoByDeviceName(const std::string &device_name)
{
  for (size_t n = 0; n < camera_info.size(); n++) {
    if (camera_info[n].device_name == device_name) {
      return &camera_info[n];
    }
  }
  return NULL;
}

/* One and only one camera service. */
std::shared_ptr<CameraService> CameraService::get() {
  static std::shared_ptr<CameraService> _instance(new CameraService());

  return _instance;
}

}