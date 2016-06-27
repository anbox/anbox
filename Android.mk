LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libprocess-cpp-minimal
LOCAL_SRC_FILES := \
    external/process-cpp-minimal/src/core/posix/process.cpp \
    external/process-cpp-minimal/src/core/posix/process_group.cpp \
    external/process-cpp-minimal/src/core/posix/signal.cpp \
    external/process-cpp-minimal/src/core/posix/signalable.cpp \
    external/process-cpp-minimal/src/core/posix/standard_stream.cpp \
    external/process-cpp-minimal/src/core/posix/wait.cpp
LOCAL_CFLAGS := \
    -fexceptions
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/external/process-cpp-minimal/include
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE := anboxd
LOCAL_SRC_FILES := \
    android/service/main.cpp \
    android/service/daemon.cpp \
    android/service/host_connector.cpp \
    android/service/local_socket_connection.cpp \
    android/service/message_processor.cpp \
    src/anbox/common/fd.cpp \
    src/anbox/bridge/message_processor.cpp \
    src/anbox/bridge/pending_call_cache.cpp \
    src/anbox/bridge/rpc_channel.cpp \
    src/anbox/protobuf/bridge.proto
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
