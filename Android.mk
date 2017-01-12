LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libprocess-cpp-minimal
LOCAL_SRC_FILES := \
    external/process-cpp-minimal/src/core/posix/process.cpp \
    external/process-cpp-minimal/src/core/posix/process_group.cpp \
    external/process-cpp-minimal/src/core/posix/signal.cpp \
    external/process-cpp-minimal/src/core/posix/signalable.cpp \
    external/process-cpp-minimal/src/core/posix/standard_stream.cpp \
    external/process-cpp-minimal/src/core/posix/wait.cpp \
    external/process-cpp-minimal/src/core/posix/fork.cpp \
    external/process-cpp-minimal/src/core/posix/exec.cpp \
    external/process-cpp-minimal/src/core/posix/child_process.cpp
LOCAL_CFLAGS := \
    -DANDROID \
    -fexceptions
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/external/process-cpp-minimal/include
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE := anboxd
LOCAL_INIT_RC := android/anboxd.rc
LOCAL_SRC_FILES := \
    android/service/main.cpp \
    android/service/daemon.cpp \
    android/service/host_connector.cpp \
    android/service/local_socket_connection.cpp \
    android/service/message_processor.cpp \
    android/service/activity_manager_interface.cpp \
    android/service/android_api_skeleton.cpp \
    android/service/platform_service_interface.cpp \
    android/service/platform_service.cpp \
    android/service/platform_api_stub.cpp \
    src/anbox/common/fd.cpp \
    src/anbox/common/wait_handle.cpp \
    src/anbox/rpc/message_processor.cpp \
    src/anbox/rpc/pending_call_cache.cpp \
    src/anbox/rpc/channel.cpp \
    src/anbox/protobuf/anbox_rpc.proto \
    src/anbox/protobuf/anbox_bridge.proto
proto_header_dir := $(call local-generated-sources-dir)/proto/$(LOCAL_PATH)/src/anbox/protobuf
LOCAL_C_INCLUDES += \
    $(proto_header_dir) \
    $(LOCAL_PATH)/external/process-cpp-minimal/include \
    $(LOCAL_PATH)/src \
    $(LOCAL_PATH)/android/service
LOCAL_EXPORT_C_INCLUDE_DIRS += $(proto_header_dir)
LOCAL_STATIC_LIBRARIES := \
    libprocess-cpp-minimal
LOCAL_SHARED_LIBRARIES := \
    liblog \
    libprotobuf-cpp-lite \
    libsysutils \
    libbinder \
    libcutils \
    libutils
LOCAL_CFLAGS := \
    -fexceptions \
    -std=c++1y
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SHARED_LIBRARIES := \
    liblog \
    lib_renderControl_enc \
    libOpenglSystemCommon
LOCAL_SRC_FILES := \
    android/hwcomposer/hwcomposer.cpp
LOCAL_MODULE := hwcomposer.anbox
LOCAL_CFLAGS:= -DLOG_TAG=\"hwcomposer\"
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/android/opengl/host/include/libOpenglRender \
    $(LOCAL_PATH)/android/opengl/shared/OpenglCodecCommon \
    $(LOCAL_PATH)/android/opengl/system/renderControl_enc \
    $(LOCAL_PATH)/android/opengl/system/OpenglSystemCommon
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audio.primary.goldfish
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libcutils liblog
LOCAL_SRC_FILES := \
    android/audio/audio_hw.cpp
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/src
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_CFLAGS := -Wno-unused-parameter

include $(BUILD_SHARED_LIBRARY)

# Include the Android.mk files below will override LOCAL_PATH so we
# have to take a copy of it here.
TMP_PATH := $(LOCAL_PATH)

include $(TMP_PATH)/android/appmgr/Android.mk
include $(TMP_PATH)/android/fingerprint/Android.mk
include $(TMP_PATH)/android/power/Android.mk
include $(TMP_PATH)/android/qemu-props/Android.mk
include $(TMP_PATH)/android/qemud/Android.mk
include $(TMP_PATH)/android/sensors/Android.mk
include $(TMP_PATH)/android/opengl/Android.mk
include $(TMP_PATH)/android/gps/Android.mk
include $(TMP_PATH)/android/lights/Android.mk
include $(TMP_PATH)/android/camera/Android.mk
include $(TMP_PATH)/android/vibrator/Android.mk
