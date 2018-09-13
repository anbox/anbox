#
# Copyright (C) 2013 The Android Open-Source Project
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
#

PRODUCT_PACKAGES += \
	egl.cfg \
	gralloc.goldfish \
	libGLESv1_CM_emulation \
	lib_renderControl_enc \
	libEGL_emulation \
	libGLES_android \
	libGLESv2_enc \
	libOpenglSystemCommon \
	libGLESv2_emulation \
	libGLESv1_enc \
	qemu-props \
	qemud \
	camera.goldfish \
	camera.goldfish.jpeg \
	lights.goldfish \
	gps.goldfish \
	fingerprint.goldfish \
	sensors.goldfish \
	audio.primary.goldfish \
	vibrator.goldfish \
	power.goldfish \
	fingerprintd

PRODUCT_COPY_FILES += \
	vendor/anbox/android/fstab.goldfish:root/fstab.goldfish \
	vendor/anbox/android/init.goldfish.rc:root/init.goldfish.rc \
	vendor/anbox/android/init.goldfish.sh:system/etc/init.goldfish.sh \
	vendor/anbox/android/ueventd.goldfish.rc:root/ueventd.goldfish.rc \
	vendor/anbox/android/media/media_profiles.xml:system/etc/media_profiles.xml \
	vendor/anbox/android/media/media_codecs.xml:system/etc/media_codecs.xml \
	vendor/anbox/android/media/media_codecs_google_audio.xml:system/etc/media_codecs_google_audio.xml \
	vendor/anbox/android/media/media_codecs_google_telephony.xml:system/etc/media_codecs_google_telephony.xml \
	vendor/anbox/android/media/media_codecs_google_tv.xml:system/etc/media_codecs_google_tv.xml \
	vendor/anbox/android/media/media_codecs_google_video.xml:system/etc/media_codecs_google_video.xml \
	hardware/libhardware_legacy/audio/audio_policy.conf:system/etc/audio_policy.conf

PRODUCT_CHARACTERISTICS := emulator

# Include drawables for all densities
PRODUCT_AAPT_CONFIG := normal

PRODUCT_COPY_FILES += \
	vendor/anbox/scripts/anbox-init.sh:root/anbox-init.sh \
	vendor/anbox/products/anbox.xml:system/etc/permissions/anbox.xml

PRODUCT_PACKAGES += \
	anboxd \
	hwcomposer.anbox \
	AnboxAppMgr

PRODUCT_PROPERTY_OVERRIDES += \
	ro.hardware=goldfish \
	ro.hardware.hwcomposer=anbox \
	ro.kernel.qemu.gles=1 \
	ro.kernel.qemu=1
	ro.adb.qemud=1

# Disable any software key elements in the UI
PRODUCT_PROPERTY_OVERRIDES += \
	qemu.hw.mainkeys=1

# Let everything know we're running inside a container
PRODUCT_PROPERTY_OVERRIDES += \
	ro.anbox=1 \
	ro.boot.container=1

# We don't want telephony support for now
PRODUCT_PROPERTY_OVERRIDES += \
	ro.radio.noril=yes

# Disable boot-animation permanently
PRODUCT_PROPERTY_OVERRIDES += \
	debug.sf.nobootanimation=1

DEVICE_PACKAGE_OVERLAYS += \
	vendor/anbox/products/overlay

$(call inherit-product, $(SRC_TARGET_DIR)/product/aosp_base.mk)
# Extend heap size we use for dalvik/art runtime
$(call inherit-product, frameworks/native/build/tablet-10in-xhdpi-2048-dalvik-heap.mk)

PRODUCT_COPY_FILES += \
	vendor/anbox/products/anbox.xml:system/etc/permissions/anbox.xml
