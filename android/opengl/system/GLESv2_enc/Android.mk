LOCAL_PATH := $(call my-dir)

### GLESv2_enc Encoder ###########################################
$(call emugl-begin-shared-library,libGLESv2_enc)

LOCAL_SRC_FILES := \
    GL2EncoderUtils.cpp \
    GL2Encoder.cpp \
    gl2_client_context.cpp \
    gl2_enc.cpp \
    gl2_entry.cpp

LOCAL_CFLAGS += -DLOG_TAG=\"emuglGLESv2_enc\"

$(call emugl-export,C_INCLUDES,$(LOCAL_PATH))
$(call emugl-import,libOpenglCodecCommon)

$(call emugl-end-module)


