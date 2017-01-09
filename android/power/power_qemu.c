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

#define LOG_TAG "emulator PowerHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>
#include <hardware/qemud.h>
#include <fcntl.h>
#include <errno.h>

static int qemud_fd;

static void power_qemu_init(struct power_module *module)
{
    qemud_fd = qemud_channel_open("hw-control");

    if (qemud_fd < 0)
        ALOGE("Error connecting to qemud hw-control service\n");
}

static void power_qemu_set_interactive(struct power_module *module, int on)
{
    int r;

    r = qemud_channel_send(qemud_fd, on ? "power:screen_state:wake"
                           : "power:screen_state:standby", -1);

    if (r < 0)
        ALOGE("Error sending power command to qemud hw-control service\n");
}

static struct hw_module_methods_t power_qemu_module_methods = {
    .open = NULL,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "Emulator Power HAL",
        .author = "The Android Open Source Project",
        .methods = &power_qemu_module_methods,
    },

    .init = power_qemu_init,
    .setInteractive = power_qemu_set_interactive,
};
