# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# this file is used to build emulator-specific program tools
# that should only run in the emulator.
#

LOCAL_PATH := $(call my-dir)

# The 'qemu-props' program is run from /system/etc/init.goldfish.rc
# to setup various system properties sent by the emulator program.
#
include $(CLEAR_VARS)
LOCAL_MODULE    := qemu-props
LOCAL_SRC_FILES := qemu-props.c
LOCAL_SHARED_LIBRARIES := libcutils liblog
include $(BUILD_EXECUTABLE)
