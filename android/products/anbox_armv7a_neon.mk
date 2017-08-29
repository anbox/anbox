#
# Copyright (C) 2013 The Android Open-Source Project
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
#

$(call inherit-product, $(LOCAL_PATH)/armv7-a-neon/device.mk)
$(call inherit-product, $(LOCAL_PATH)/anbox.mk)

# Enable low-mem related options (see https://source.android.com/devices/tech/config/low-ram)
PRODUCT_PROPERTY_OVERRIDES += ro.config.low_ram=true
PRODUCT_PROPERTY_OVERRIDES += dalvik.vm.jit.codecachesize=0

PRODUCT_NAME := anbox_armv7a_neon
PRODUCT_DEVICE := armv7-a-neon
PRODUCT_BRAND := Android
PRODUCT_MODEL := Anbox
