LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_JAVA_LIBRARIES := \
	android-common \
	android-support-v13
LOCAL_SRC_FILES := $(call all-java-files-under, src)
# LOCAL_SDK_VERSION := current
LOCAL_PACKAGE_NAME := AnboxAppMgr
LOCAL_CERTIFICATE := shared
LOCAL_PRIVILEGED_MODULE := true
LOCAL_OVERRIDES_PACKAGES := \
	Home \
	Launcher2 \
	Launcher3 \
	LatinIME \
	QuickSearchBox
include $(BUILD_PACKAGE)
