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

$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(LOCAL_PATH)/x86_64/device.mk)

$(call inherit-product, $(LOCAL_PATH)/anbox.mk)

PRODUCT_NAME := anbox_x86_64
PRODUCT_DEVICE := x86_64
PRODUCT_BRAND := Android
PRODUCT_MODEL := Anbox
