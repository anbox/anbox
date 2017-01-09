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
 * Contains implementation of the camera HAL layer in the system running
 * under the emulator.
 *
 * This file contains only required HAL header, which directs all the API calls
 * to the EmulatedCameraFactory class implementation, wich is responsible for
 * managing emulated cameras.
 */

#include "EmulatedCameraFactory.h"

/*
 * Required HAL header.
 */
camera_module_t HAL_MODULE_INFO_SYM = {
    common: {
         tag:                HARDWARE_MODULE_TAG,
         module_api_version: CAMERA_MODULE_API_VERSION_2_3,
         hal_api_version:    HARDWARE_HAL_API_VERSION,
         id:                 CAMERA_HARDWARE_MODULE_ID,
         name:               "Emulated Camera Module",
         author:             "The Android Open Source Project",
         methods:            &android::EmulatedCameraFactory::mCameraModuleMethods,
         dso:                NULL,
         reserved:           {0},
    },
    get_number_of_cameras:  android::EmulatedCameraFactory::get_number_of_cameras,
    get_camera_info:        android::EmulatedCameraFactory::get_camera_info,
    set_callbacks:          android::EmulatedCameraFactory::set_callbacks,
    get_vendor_tag_ops:     android::EmulatedCameraFactory::get_vendor_tag_ops,
    open_legacy:            android::EmulatedCameraFactory::open_legacy
};
