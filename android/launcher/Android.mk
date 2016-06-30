LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
	$(call all-java-files-under src)
LOCAL_SDK_VERSION := current
LOCAL_PACKAGE_NAME := AnboxLauncher
LOCAL_CERTIFICATE := shared
LOCAL_PRIVILEGED_MODULE := true
LOCAL_OVERRIDES_PACKAGES := Home
include $(BUILD_PACKAGE)
