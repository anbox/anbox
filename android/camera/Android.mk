# Copyright (C) 2011 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Emulator camera module########################################################

emulator_camera_module_relative_path := hw
emulator_camera_cflags := -fno-short-enums -DQEMU_HARDWARE
emulator_camera_cflags += -Wno-unused-parameter -Wno-missing-field-initializers
emulator_camera_clang_flags := -Wno-c++11-narrowing
emulator_camera_shared_libraries := \
    libbinder \
    liblog \
    libutils \
    libcutils \
    libcamera_client \
    libui \
    libdl \
	libjpeg \
	libcamera_metadata

emulator_camera_c_includes := external/jpeg \
	frameworks/native/include/media/hardware \
	$(LOCAL_PATH)/../opengl/system/OpenglSystemCommon \
	$(call include-path-for, camera)

emulator_camera_src := \
	EmulatedCameraHal.cpp \
	EmulatedCameraFactory.cpp \
	EmulatedCameraHotplugThread.cpp \
	EmulatedBaseCamera.cpp \
	EmulatedCamera.cpp \
		EmulatedCameraDevice.cpp \
		EmulatedQemuCamera.cpp \
		EmulatedQemuCameraDevice.cpp \
		EmulatedFakeCamera.cpp \
		EmulatedFakeCameraDevice.cpp \
		Converters.cpp \
		PreviewWindow.cpp \
		CallbackNotifier.cpp \
		QemuClient.cpp \
		JpegCompressor.cpp \
	EmulatedCamera2.cpp \
		EmulatedFakeCamera2.cpp \
		EmulatedQemuCamera2.cpp \
		fake-pipeline2/Scene.cpp \
		fake-pipeline2/Sensor.cpp \
		fake-pipeline2/JpegCompressor.cpp \
	EmulatedCamera3.cpp \
		EmulatedFakeCamera3.cpp

# Emulated camera - goldfish / vbox_x86 build###################################

LOCAL_MODULE_RELATIVE_PATH := ${emulator_camera_module_relative_path}
LOCAL_CFLAGS := ${emulator_camera_cflags}
LOCAL_CLANG_CFLAGS += ${emulator_camera_clang_flags}

LOCAL_SHARED_LIBRARIES := ${emulator_camera_shared_libraries}
LOCAL_C_INCLUDES += ${emulator_camera_c_includes}
LOCAL_SRC_FILES := ${emulator_camera_src}

ifeq ($(TARGET_PRODUCT),vbox_x86)
LOCAL_MODULE := camera.vbox_x86
else
LOCAL_MODULE := camera.goldfish
endif

include $(BUILD_SHARED_LIBRARY)

# Emulator camera - ranchu build################################################

include ${CLEAR_VARS}

LOCAL_MODULE_RELATIVE_PATH := ${emulator_camera_module_relative_path}
LOCAL_CFLAGS := ${emulator_camera_cflags}
LOCAL_CLANG_CFLAGS += ${emulator_camera_clang_flags}

LOCAL_SHARED_LIBRARIES := ${emulator_camera_shared_libraries}
LOCAL_C_INCLUDES += ${emulator_camera_c_includes}
LOCAL_SRC_FILES := ${emulator_camera_src}

LOCAL_MODULE := camera.ranchu

include $(BUILD_SHARED_LIBRARY)

# JPEG stub#####################################################################

ifneq ($(TARGET_BUILD_PDK),true)

include $(CLEAR_VARS)

jpeg_module_relative_path := hw
jpeg_cflags := -fno-short-enums -DQEMU_HARDWARE
jpeg_cflags += -Wno-unused-parameter
jpeg_clang_flags += -Wno-c++11-narrowing
jpeg_shared_libraries := \
    libcutils \
    liblog \
    libskia \
    libandroid_runtime
jpeg_c_includes := external/libjpeg-turbo \
                   external/skia/include/core/ \
                   frameworks/base/core/jni/android/graphics \
                   frameworks/native/include
jpeg_src := JpegStub.cpp

# JPEG stub - goldfish build####################################################

LOCAL_MODULE_RELATIVE_PATH := ${jpeg_module_relative_path}
LOCAL_CFLAGS += ${jpeg_cflags}
LOCAL_CLANG_CFLAGS += ${jpeg_clangflags}

LOCAL_SHARED_LIBRARIES := ${jpeg_shared_libraries}
LOCAL_C_INCLUDES += ${jpeg_c_includes}
LOCAL_SRC_FILES := ${jpeg_src}

LOCAL_MODULE := camera.goldfish.jpeg

include $(BUILD_SHARED_LIBRARY)

# JPEG stub - ranchu build######################################################

include ${CLEAR_VARS}

LOCAL_MODULE := camera.ranchu.jpeg

LOCAL_MODULE_RELATIVE_PATH := ${jpeg_module_relative_path}
LOCAL_CFLAGS += ${jpeg_cflags}
LOCAL_CLANG_CFLAGS += ${jpeg_clangflags}

LOCAL_SHARED_LIBRARIES := ${jpeg_shared_libraries}
LOCAL_C_INCLUDES += ${jpeg_c_includes}
LOCAL_SRC_FILES := ${jpeg_src}

include $(BUILD_SHARED_LIBRARY)

endif # !PDK
