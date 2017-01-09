LOCAL_PATH := $(call my-dir)

$(call emugl-begin-shared-library,libut_rendercontrol_enc)
$(call emugl-import,libOpenglCodecCommon)
$(call emugl-gen-encoder,$(LOCAL_PATH),ut_rendercontrol)
$(call emugl-export,C_INCLUDES,$(LOCAL_PATH))
$(call emugl-end-module)

