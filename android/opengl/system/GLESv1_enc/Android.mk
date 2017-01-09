LOCAL_PATH := $(call my-dir)

### GLESv1_enc Encoder ###########################################
$(call emugl-begin-shared-library,libGLESv1_enc)

LOCAL_CFLAGS += -DLOG_TAG=\"emuglGLESv1_enc\"

LOCAL_SRC_FILES := \
        GLEncoder.cpp \
        GLEncoderUtils.cpp \
        gl_client_context.cpp \
        gl_enc.cpp \
        gl_entry.cpp

$(call emugl-import,libOpenglCodecCommon)
$(call emugl-export,C_INCLUDES,$(LOCAL_PATH))
$(call emugl-export,C_INCLUDES,$(intermediates))

$(call emugl-end-module)
