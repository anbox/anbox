# Copyright (C) 2011 The Android Open Source Project
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

THIS_LOCAL_PATH := $(realpath $(call my-dir))
LOCAL_PATH := $(ANDROID_BUILD_TOP)/frameworks/opt/net/wifi/service

# Partwise taken from frameworks/opt/net/wifi/service/Android.mk
# ============================================================

ifneq ($(TARGET_BUILD_PDK), true)

include $(CLEAR_VARS)

PATCHED_STAMP_FILE := $(ANDROID_HOST_OUT)/.wsm_patched.stamp
TO_BE_PATCHED_DIR := $(ANDROID_BUILD_TOP)/frameworks/opt/net/wifi
CHECK_IF_PATCHED_FILE := $(LOCAL_PATH)/java/com/android/server/wifi/WifiStateMachine.java
PREVIOUS_HASH := $(shell grep -hs ^ $(PATCHED_STAMP_FILE))

LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/java
LOCAL_SRC_FILES := $(call all-java-files-under, java) \
	$(call all-Iaidl-files-under, java) \
	$(call all-logtags-files-under, java) \
	$(call all-proto-files-under, proto) \
	$(PATCHED_STAMP_FILE)

$(PATCHED_STAMP_FILE): $(CHECK_IF_PATCHED_FILE)

$(CHECK_IF_PATCHED_FILE):
	git -C $(TO_BE_PATCHED_DIR) rev-parse HEAD > $(PATCHED_STAMP_FILE)
	git -C $(TO_BE_PATCHED_DIR) am $(THIS_LOCAL_PATH)/wifiStateMachine.patch

ifndef INCLUDE_NAN_FEATURE
LOCAL_SRC_FILES := $(filter-out $(call all-java-files-under, \
	java/com/android/server/wifi/nan),$(LOCAL_SRC_FILES))
endif

ifdef PREVIOUS_HASH
$(call add-clean-step, git -C $(TO_BE_PATCHED_DIR) reset --hard $(PREVIOUS_HASH))
$(call add-clean-step, rm $(PATCHED_STAMP_FILE))
endif

LOCAL_JAVA_LIBRARIES := bouncycastle conscrypt services
LOCAL_REQUIRED_MODULES := services
LOCAL_MODULE_TAGS :=
LOCAL_MODULE := wifi-service-anbox
LOCAL_OVERRIDES_MODULE := wifi-service
LOCAL_PROTOC_OPTIMIZE_TYPE := nano

# Protoc uses proto_path=., but wifi.proto is not here
LOCAL_PROTOC_FLAGS := --proto_path=$(LOCAL_PATH)

ifeq ($(EMMA_INSTRUMENT_FRAMEWORK),true)
LOCAL_EMMA_INSTRUMENT := true
endif

LOCAL_JACK_COVERAGE_INCLUDE_FILTER := com.android.server.wifi.*

include $(BUILD_JAVA_LIBRARY)

endif
