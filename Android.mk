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
LOCAL_SRC_FILES := \
    android/service/main.cpp \
    android/service/daemon.cpp \
    android/service/host_connector.cpp \
    android/service/local_socket_connection.cpp \
    android/service/message_processor.cpp \
    android/service/platform_api.cpp \
    src/anbox/common/fd.cpp \
    src/anbox/bridge/message_processor.cpp \
    src/anbox/bridge/pending_call_cache.cpp \
    src/anbox/bridge/rpc_channel.cpp \
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
    libsysutils
LOCAL_CFLAGS := \
    -fexceptions \
    -std=c++1y
include $(BUILD_EXECUTABLE)

# Include the Android.mk files below will override LOCAL_PATH so we
# have to take a copy of it here.
TMP_PATH := $(LOCAL_PATH)

# The compositor and launcher have their own Android.mk in their subfolders
# as they should not depend on any other anbox sources.
# include $(LOCAL_PATH)/android/shared_compositor/Android.mk
include $(TMP_PATH)/android/launcher/Android.mk
include $(TMP_PATH)/android/shared_compositor/Android.mk
