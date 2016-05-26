LOCAL_PATH := $(call my-dir)

### host library #########################################
$(call emugl-begin-host-static-library,libGLESv1_dec)

$(call emugl-import, libOpenglCodecCommon)
$(call emugl-export,C_INCLUDES,$(LOCAL_PATH))

$(call emugl-gen-decoder,$(LOCAL_PATH),gles1)

LOCAL_SRC_FILES := GLESv1Decoder.cpp

$(call emugl-export,CFLAGS,$(EMUGL_USER_CFLAGS))
$(call emugl-export,LDLIBS,$(CXX_STD_LIB))

$(call emugl-end-module)
