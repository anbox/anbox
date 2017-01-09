# This is the top-level build file for the Android HW OpenGL ES emulation
# in Android.
#
# You must define BUILD_EMULATOR_OPENGL to 'true' in your environment to
# build the following files.
#
# Also define BUILD_EMULATOR_OPENGL_DRIVER to 'true' to build the gralloc
# stuff as well.
#
ifeq (true,$(BUILD_EMULATOR_OPENGL))

# Top-level for all modules
EMUGL_PATH := $(call my-dir)

# Directory containing common headers used by several modules
# This is always set to a module's LOCAL_C_INCLUDES
# See the definition of emugl-begin-module in common.mk
#
EMUGL_COMMON_INCLUDES := $(EMUGL_PATH)/host/include/libOpenglRender

# common cflags used by several modules
# This is always set to a module's LOCAL_CFLAGS
# See the definition of emugl-begin-module in common.mk
#
EMUGL_COMMON_CFLAGS := -DWITH_GLES2

# Uncomment the following line if you want to enable debug traces
# in the GLES emulation libraries.
# EMUGL_COMMON_CFLAGS += -DEMUGL_DEBUG=1

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

include $(EMUGL_PATH)/shared/OpenglCodecCommon/Android.mk

# System static libraries
include $(EMUGL_PATH)/system/GLESv1_enc/Android.mk
include $(EMUGL_PATH)/system/GLESv2_enc/Android.mk
include $(EMUGL_PATH)/system/renderControl_enc/Android.mk
include $(EMUGL_PATH)/system/OpenglSystemCommon/Android.mk

# System shared libraries
include $(EMUGL_PATH)/system/GLESv1/Android.mk
include $(EMUGL_PATH)/system/GLESv2/Android.mk

include $(EMUGL_PATH)/system/gralloc/Android.mk
include $(EMUGL_PATH)/system/egl/Android.mk

endif # BUILD_EMULATOR_OPENGL == true
