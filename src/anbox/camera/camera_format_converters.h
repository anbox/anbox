/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Adapted for Anbox by Alec Barber 2021
 */

#ifndef ANDROID_CAMERA_CAMERA_FORMAT_CONVERTERS_H
#define ANDROID_CAMERA_CAMERA_FORMAT_CONVERTERS_H

/*
 * Contains declaration of the API that allows converting frames from one
 * pixel format to another.
 *
 * For the emulator, we really need to convert into two formats: YV12, which is
 * used by the camera framework for video, and RGB32 for preview window.
 */

#include "camera_common.h"

namespace anbox::camera {

/* Checks if conversion between two pixel formats is available.
 * Param:
 *  from - Pixel format to convert from.
 *  to - Pixel format to convert to.
 * Return:
 *  boolean: 1 if converter is available, or 0 if no conversion exists.
 */
int has_converter(uint32_t from, uint32_t to);

/* Converts a frame into multiple framebuffers.
 * When camera service replies to a framebuffer request from the client, it
 * usualy sends two framebuffers in the reply: one for video, and another for
 * preview window. Since these two framebuffers have different pixel formats
 * (most of the time), we need to do two conversions for each frame received from
 * the camera. This is the main intention behind this routine: to have a one call
 * that produces as many conversions as needed.
 * Param:
 *  frame - Frame to convert.
 *  pixel_format - Defines pixel format for the converting framebuffer.
 *  framebuffer_size, width, height - Converting framebuffer byte size, width,
 *      and height.
 *  framebuffers - Array of framebuffers where to convert the frame. Size of this
 *      array is defined by the 'fbs_num' parameter. Note that the caller must
 *      make sure that buffers are large enough to contain entire frame captured
 *      from the device.
 *  fbs_num - Number of entries in the 'framebuffers' array.
 *  r_scale, g_scale, b_scale - White balance scale.
 *  exp_comp - Expsoure compensation.
 * Return:
 *  0 on success, or non-zero value on failure.
 */
int convert_frame(const void* frame,
                  uint32_t pixel_format,
                  size_t framebuffer_size,
                  int width,
                  int height,
                  ClientFrameBuffer* framebuffers,
                  int fbs_num,
                  float r_scale,
                  float g_scale,
                  float b_scale,
                  float exp_comp);

}

#endif  /* ANDROID_CAMERA_CAMERA_FORMAT_CONVERTERS_H */