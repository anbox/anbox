LOCAL_PATH := $(call my-dir)

$(call emugl-begin-shared-library,lib_renderControl_enc)

LOCAL_SRC_FILES := \
    renderControl_client_context.cpp \
    renderControl_enc.cpp \
    renderControl_entry.cpp

$(call emugl-export,C_INCLUDES,$(LOCAL_PATH))
$(call emugl-import,libOpenglCodecCommon)
$(call emugl-end-module)
