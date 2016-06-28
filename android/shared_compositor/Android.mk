LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE := anbox_shared_compositor
LOCAL_SRC_FILES := \
    main.cpp \
    surface_composer.cpp
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libdl \
    libhardware \
    libutils \
    libEGL \
    libGLESv1_CM \
    libGLESv2 \
    libbinder \
    libui \
    libgui
LOCAL_CFLAGS := \
    -fexceptions \
    -std=c++1y
include $(BUILD_EXECUTABLE)
