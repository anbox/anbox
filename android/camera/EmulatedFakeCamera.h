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

#ifndef HW_EMULATOR_CAMERA_EMULATED_FAKE_CAMERA_H
#define HW_EMULATOR_CAMERA_EMULATED_FAKE_CAMERA_H

/*
 * Contains declaration of a class EmulatedFakeCamera that encapsulates
 * functionality of a fake camera. This class is nothing more than a placeholder
 * for EmulatedFakeCameraDevice instance.
 */

#include "EmulatedCamera.h"
#include "EmulatedFakeCameraDevice.h"

namespace android {

/* Encapsulates functionality of a fake camera.
 * This class is nothing more than a placeholder for EmulatedFakeCameraDevice
 * instance that emulates a fake camera device.
 */
class EmulatedFakeCamera : public EmulatedCamera {
public:
    /* Constructs EmulatedFakeCamera instance. */
    EmulatedFakeCamera(int cameraId, bool facingBack, struct hw_module_t* module);

    /* Destructs EmulatedFakeCamera instance. */
    ~EmulatedFakeCamera();

    /****************************************************************************
     * EmulatedCamera virtual overrides.
     ***************************************************************************/

public:
    /* Initializes EmulatedFakeCamera instance. */
     status_t Initialize();

    /****************************************************************************
     * EmulatedCamera abstract API implementation.
     ***************************************************************************/

protected:
    /* Gets emulated camera device ised by this instance of the emulated camera.
     */
    EmulatedCameraDevice* getCameraDevice();

    /****************************************************************************
     * Data memebers.
     ***************************************************************************/

protected:
    /* Facing back (true) or front (false) switch. */
    bool                        mFacingBack;

    /* Contained fake camera device object. */
    EmulatedFakeCameraDevice    mFakeCameraDevice;
};

}; /* namespace android */

#endif  /* HW_EMULATOR_CAMERA_EMULATED_FAKE_CAMERA_H */
