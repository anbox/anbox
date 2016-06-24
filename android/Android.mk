LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES = \
	jni/anbox_support.cpp \
	jni/application_manager.cpp \
	jni/jni_helper.cpp
LOCAL_C_INCLUDES := \
	$(JNI_H_INCLUDE) \
	libcore/include
LOCAL_SHARED_LIBRARIES := \
	liblog \
	libnativehelper
LOCAL_MODULE := libanbox_support

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/java
LOCAL_SRC_FILES := $(call all-java-files-under, java)
LOCAL_JAVA_LIBRARIES := telephony-common
LOCAL_JNI_SHARED_LIBRARIES := \
	libanbox_support
# Block certain packages from being installed
# LOCAL_OVERRIDES_PACKAGES := \
# 	SystemUI \
# 	Home \
# 	Launcher2 \
# 	Calculator \
# 	BasicDreams \
# 	Calendar \
# 	PrintSpooler \
# 	WallpaperCropper
LOCAL_PACKAGE_NAME := anboxd
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
