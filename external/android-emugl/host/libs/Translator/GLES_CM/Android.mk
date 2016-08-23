LOCAL_PATH := $(call my-dir)

host_common_SRC_FILES := \
     GLEScmImp.cpp       \
     GLEScmUtils.cpp     \
     GLEScmContext.cpp   \
     GLEScmValidate.cpp


### GLES_CM host implementation (On top of OpenGL) ########################
$(call emugl-begin-host-shared-library,lib$(BUILD_TARGET_SUFFIX)GLES_CM_translator)
$(call emugl-import,libGLcommon)
LOCAL_SRC_FILES := $(host_common_SRC_FILES)
$(call emugl-end-module)
