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

#ifndef HW_EMULATOR_CAMERA_EMULATED_QEMU_CAMERA2_H
#define HW_EMULATOR_CAMERA_EMULATED_QEMU_CAMERA2_H

/*
 * Contains declaration of a class EmulatedQemuCamera2 that encapsulates
 * functionality of a host webcam with added processing to implement version 2
 * of the camera device interface.
 */

#include "EmulatedCamera2.h"

namespace android {

/* Encapsulates functionality of an advanced fake camera based on real host camera data.
 */
class EmulatedQemuCamera2 : public EmulatedCamera2 {
public:
    /* Constructs EmulatedFakeCamera instance. */
    EmulatedQemuCamera2(int cameraId, bool facingBack, struct hw_module_t* module);

    /* Destructs EmulatedFakeCamera instance. */
    ~EmulatedQemuCamera2();

    /****************************************************************************
     * EmulatedCamera2 virtual overrides.
     ***************************************************************************/

public:
    /* Initializes EmulatedQemuCamera2 instance. */
     status_t Initialize();

    /****************************************************************************
     * EmulatedCamera abstract API implementation.
     ***************************************************************************/

protected:

    /****************************************************************************
     * Data memebers.
     ***************************************************************************/

protected:
    /* Facing back (true) or front (false) switch. */
    bool                        mFacingBack;

};

}; /* namespace android */

#endif  /* HW_EMULATOR_CAMERA_EMULATED_QEMU_CAMERA2_H */
