/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <errno.h>
#include <stdlib.h>

#include <cutils/log.h>

#define QEMU_HARDWARE
#include <qemu.h>
#include <hardware/hardware.h>
#include <hardware/vibrator.h>

static int sendit(unsigned int timeout_ms)
{
    if (qemu_check()) {
        if (qemu_control_command("vibrator:%u", timeout_ms) < 0) {
            return -errno;
        }
        return 0;
    }

    return -ENOSYS;
}

static int qemu_vibra_on(vibrator_device_t* vibradev __unused, unsigned int timeout_ms)
{
    return sendit(timeout_ms);
}

static int qemu_vibra_off(vibrator_device_t* vibradev __unused)
{
    return sendit(0);
}

static int qemu_vibra_close(hw_device_t *device)
{
    free(device);
    return 0;
}

static int qemu_vibra_open(const hw_module_t* module, const char* id __unused,
                           hw_device_t** device) {
    vibrator_device_t *vibradev = calloc(1, sizeof(vibrator_device_t));

    if (!vibradev) {
        ALOGE("No memory available to create Goldfish vibrator device!");
        return -ENOMEM;
    }

    vibradev->common.tag = HARDWARE_DEVICE_TAG;
    vibradev->common.module = (hw_module_t *) module;
    vibradev->common.version = HARDWARE_DEVICE_API_VERSION(1,0);
    vibradev->common.close = qemu_vibra_close;

    vibradev->vibrator_on = qemu_vibra_on;
    vibradev->vibrator_off = qemu_vibra_off;

    *device = (hw_device_t *) vibradev;

    return 0;
}

/*===========================*/
/* Emulator vibrator module  */
/*===========================*/

static struct hw_module_methods_t qemu_vibrator_module_methods = {
    .open = qemu_vibra_open,
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .module_api_version = VIBRATOR_API_VERSION,
    .hal_api_version = HARDWARE_HAL_API_VERSION,
    .id = VIBRATOR_HARDWARE_MODULE_ID,
    .name = "Goldfish vibrator module",
    .author = "The Android Open Source Project",
    .methods = &qemu_vibrator_module_methods,
};
