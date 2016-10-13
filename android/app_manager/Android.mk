LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_PACKAGE_NAME := AnboxAppManager
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true
# We kick out several core packages here which would
# otherwise put up the base UI we don't want.
LOCAL_OVERRIDES_PACKAGES := \
	Home \
	Launcher2 \
	Launcher3
include $(BUILD_PACKAGE)
