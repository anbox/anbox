# This build script corresponds to a library containing many definitions
# common to both the guest and the host. They relate to
#
LOCAL_PATH := $(call my-dir)

commonSources := \
        GLClientState.cpp \
        ChecksumCalculator.cpp \
        GLSharedGroup.cpp \
        glUtils.cpp \
        SocketStream.cpp \
        TcpStream.cpp \

### CodecCommon  guest ##############################################
$(call emugl-begin-static-library,libOpenglCodecCommon)

LOCAL_SRC_FILES := $(commonSources)

LOCAL_CFLAGS += -DLOG_TAG=\"eglCodecCommon\"

$(call emugl-export,SHARED_LIBRARIES,libcutils libutils liblog)
$(call emugl-export,C_INCLUDES,$(LOCAL_PATH))
$(call emugl-end-module)
