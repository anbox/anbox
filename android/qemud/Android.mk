# Copyright 2008 The Android Open Source Project

# We're moving the emulator-specific platform libs to
# development.git/tools/emulator/. The following test is to ensure
# smooth builds even if the tree contains both versions.
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	qemud.c


LOCAL_SHARED_LIBRARIES := \
	libcutils liblog

LOCAL_MODULE:= qemud

include $(BUILD_EXECUTABLE)
