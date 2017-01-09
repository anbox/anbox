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

#ifndef HW_EMULATOR_CAMERA_EMULATED_CAMERA_COMMON_H
#define HW_EMULATOR_CAMERA_EMULATED_CAMERA_COMMON_H

/*
 * Contains common declarations that are used across the camera emulation.
 */

#include <linux/videodev2.h>
#include <hardware/camera.h>

/* A helper class that tracks a routine execution.
 * Basically, it dumps an enry message in its constructor, and an exit message
 * in its destructor. Use LOGRE() macro (declared bellow) to create instances
 * of this class at the beginning of the tracked routines / methods.
 */
class HWERoutineTracker {
public:
    /* Constructor that prints an "entry" trace message. */
    explicit HWERoutineTracker(const char* name)
            : mName(name) {
        ALOGV("Entering %s", mName);
    }

    /* Destructor that prints a "leave" trace message. */
    ~HWERoutineTracker() {
        ALOGV("Leaving %s", mName);
    }

private:
    /* Stores the routine name. */
    const char* mName;
};

/* Logs an execution of a routine / method. */
#define LOGRE() HWERoutineTracker hwertracker_##__LINE__(__FUNCTION__)

/*
 * min / max macros
 */

#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define max(a,b)    (((a) > (b)) ? (a) : (b))

#endif  /* HW_EMULATOR_CAMERA_EMULATED_CAMERA_COMMON_H */
