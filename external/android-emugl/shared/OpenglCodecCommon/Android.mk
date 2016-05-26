# This build script corresponds to a library containing many definitions
# common to both the guest and the host. They relate to
#
LOCAL_PATH := $(call my-dir)

commonSources := \
        glUtils.cpp \
        ChecksumCalculator.cpp \
        ChecksumCalculatorThreadInfo.cpp \

host_commonSources := $(commonSources)

host_commonLdLibs := $(CXX_STD_LIB)

### OpenglCodecCommon  host ##############################################
$(call emugl-begin-host-static-library,libOpenglCodecCommon)

LOCAL_SRC_FILES := $(host_commonSources)
$(call emugl-import, libemugl_common)
$(call emugl-export,C_INCLUDES,$(EMUGL_PATH)/host/include/libOpenglRender $(LOCAL_PATH))
$(call emugl-export,LDLIBS,$(host_commonLdLibs))
$(call emugl-end-module)


