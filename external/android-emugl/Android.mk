# This is the top-level build file for the Android HW OpenGL ES emulation
# in Android.
#
# You must define BUILD_EMULATOR_HOST_OPENGL to 'true' in your environment to
# build the following files.
#
# Top-level for all modules
EMUGL_PATH := $(call my-dir)

EMUGL_OLD_LOCAL_PATH := $(LOCAL_PATH)

# Directory containing common headers that are part of EmuGL's public API.
# This is always set to a module's LOCAL_C_INCLUDES. See the definition of
# emugl-begin-module in common.mk
EMUGL_COMMON_INCLUDES := $(EMUGL_INCLUDES)

# common cflags used by several modules
# This is always set to a module's LOCAL_CFLAGS
# See the definition of emugl-begin-module in common.mk

# Needed to ensure SIZE_MAX is properly defined when including <stdint.h>
EMUGL_COMMON_CFLAGS += -D__STDC_LIMIT_MACROS=1

ifneq (,$(strip $(BUILD_DEBUG)))
EMUGL_COMMON_CFLAGS += -DEMUGL_DEBUG=1
endif

EMUGL_COMMON_CFLAGS += -DEMUGL_BUILD=1
ifeq (linux,$(BUILD_TARGET_OS))
EMUGL_COMMON_CFLAGS += -fvisibility=internal
endif
ifeq (darwin,$(BUILD_TARGET_OS))
EMUGL_COMMON_CFLAGS += -fvisibility=hidden
endif

# Include common definitions used by all the modules included later
# in this build file. This contains the definition of all useful
# emugl-xxxx functions.
#
include $(EMUGL_PATH)/common.mk

# IMPORTANT: ORDER IS CRUCIAL HERE
#
# For the import/export feature to work properly, you must include
# modules below in correct order. That is, if module B depends on
# module A, then it must be included after module A below.
#
# This ensures that anything exported by module A will be correctly
# be imported by module B when it is declared.
#
# Note that the build system will complain if you try to import a
# module that hasn't been declared yet anyway.
#

# Required by our units test.
include $(EMUGL_PATH)/googletest.mk

# First, build the emugen host source-generation tool
#
# It will be used by other modules to generate wire protocol encode/decoder
# source files (see all emugl-gen-decoder/encoder in common.mk)
#
include $(EMUGL_PATH)/host/tools/emugen/Android.mk

include $(EMUGL_PATH)/shared/emugl/common/Android.mk
include $(EMUGL_PATH)/shared/OpenglCodecCommon/Android.mk

ifeq (true,$(EMULATOR_USE_ANGLE))
# Alternative graphics translation (GT) implementation, stripped off
# and adjust from the equivalent mod in the ARC project.
# This GT acts as a thin wrapper + GLESv1-to-v2 translator
# and forwards GLES calls to underlying GLES API (e.g. ANGLE)
#
include $(EMUGL_PATH)/host/libs/graphics_translation/common/Android.mk
include $(EMUGL_PATH)/host/libs/graphics_translation/gles/Android.mk
endif

# Host static libraries
include $(EMUGL_PATH)/host/libs/GLESv1_dec/Android.mk
include $(EMUGL_PATH)/host/libs/GLESv2_dec/Android.mk
include $(EMUGL_PATH)/host/libs/renderControl_dec/Android.mk
include $(EMUGL_PATH)/host/libs/Translator/GLcommon/Android.mk
include $(EMUGL_PATH)/host/libs/Translator/GLES_CM/Android.mk
include $(EMUGL_PATH)/host/libs/Translator/GLES_V2/Android.mk
include $(EMUGL_PATH)/host/libs/Translator/EGL/Android.mk
include $(EMUGL_PATH)/host/libs/libOpenGLESDispatch/Android.mk

# Host shared libraries
include $(EMUGL_PATH)/host/libs/libOpenglRender/Android.mk

LOCAL_PATH := $(EMUGL_OLD_LOCAL_PATH)
