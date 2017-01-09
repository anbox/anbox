LOCAL_PATH := $(call my-dir)

#### libGLESv1_CM_emul.so
$(call emugl-begin-shared-library,libGLESv1_CM_emul)
$(call emugl-import,libGLESv1_enc)
$(call emugl-gen-wrapper,$(EMUGL_PATH)/system/GLESv1_enc,gl)
$(call emugl-set-shared-library-subpath,egl)

LOCAL_SRC_FILES += glesv1_emul_ifc.cpp

$(call emugl-end-module)

emulatorOpengl := $(LOCAL_PATH)/../..
logTag := -DLOG_TAG=\"eglWrapper\"
EMUGEN = $(BUILD_OUT_EXECUTABLES)/emugen
## comment for no debug
#debugFlags = -g -O0

#### libGLESv2_CM_emul.so
$(call emugl-begin-shared-library, libGLESv2_emul)
$(call emugl-import,libGLESv2_enc)
$(call emugl-gen-wrapper,$(EMUGL_PATH)/system/GLESv2_enc,gl2)
LOCAL_SRC_FILES += glesv2_emul_ifc.cpp
$(call emugl-set-shared-library-subpath,egl)
$(call emugl-end-module)

##### libEGL_emul.so ###########

# THE FOLLOWING DOESN'T WORK YET
#
$(call emugl-begin-shared-library,libEGL_emul)
$(call emugl-import,libut_rendercontrol_enc libGLESv1_CM_emul libGLESv2_emul libOpenglSystemCommon)

$(call emugl-set-shared-library-subpath,egl)
LOCAL_CFLAGS += $(logTag)

LOCAL_SRC_FILES :=  \
        egl.cpp \
        egl_dispatch.cpp \
        ServerConnection.cpp \
        ThreadInfo.cpp

$(call emugl-end-module)

#### egl.cfg ####

# Ensure that this file is only copied to emulator-specific builds.
# Other builds are device-specific and will provide their own
# version of this file to point to the appropriate HW EGL libraries.
#
ifneq (,$(filter full full_x86 full_mips sdk sdk_x86 sdk_mips,$(TARGET_PRODUCT)))
ifeq (,$(BUILD_EMULATOR_OPENGL_DRIVER))
include $(CLEAR_VARS)

LOCAL_MODULE := egl.cfg
LOCAL_SRC_FILES := $(LOCAL_MODULE)

LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/egl
LOCAL_MODULE_CLASS := ETC

include $(BUILD_PREBUILT)
endif # building 'real' driver BUILD_EMULATOR_OPENGL_DRIVER
endif # TARGET_PRODUCT in 'full sdk full_x86 sdk_x86 full_mips sdk_mips'

#### gles_emul.cfg ####
include $(CLEAR_VARS)

LOCAL_MODULE := gles_emul.cfg
LOCAL_SRC_FILES := $(LOCAL_MODULE)

LOCAL_MODULE_PATH := $(TARGET_OUT)/etc
LOCAL_MODULE_CLASS := ETC

include $(BUILD_PREBUILT)
