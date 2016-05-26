LOCAL_PATH := $(call my-dir)

### host library ##########################################
$(call emugl-begin-host-static-library,libOpenGLESDispatch)
$(call emugl-import,libGLESv2_dec libGLESv1_dec)

# use Translator's egl headers
LOCAL_C_INCLUDES += $(EMUGL_PATH)/host/libs/Translator/include
LOCAL_C_INCLUDES += $(EMUGL_PATH)/shared

LOCAL_SRC_FILES := EGLDispatch.cpp \
                   GLESv2Dispatch.cpp \
                   GLESv1Dispatch.cpp
$(call emugl-end-module)
