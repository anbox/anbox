/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef EGL_CONFIG_H
#define EGL_CONFIG_H

#include "EglOsApi.h"
#include <EGL/egl.h>

#include <list>

#define MIN_SWAP_INTERVAL 1
#define MAX_SWAP_INTERVAL 10

// A class used to model the content of an EGLConfig object.
// This is really a structure with several fields that can be queried
// individually with getConfigAttrib(), and compared for sorting according
// to the EGLConfig specification.
class EglConfig {
public:
    // Return the value of a given attribute, identified by its name |attrib|
    // (e.g. EGL_CONFIG_ID). On success, return true and sets |*val| to the
    // attribute value. On failure (unknown |attrib| value), return false.
    bool getConfAttrib(EGLint attrib, EGLint* val) const;

    // Comparison operators used to sort EglConfig instances.
    bool operator<(const EglConfig& conf) const;
    bool operator>=(const EglConfig& conf) const;

    // Return true iff this instance is compatible with |conf|, i.e. that
    // they have the same red/green/blue/depth/stencil sizes.
    bool compatibleWith(const EglConfig& conf)  const; //compatibility

    // Return true iff this instance matches the minimal requirements of
    // another EglConfig |dummy|. Any attribute value in |dummy| that isn't
    // EGL_DONT_CARE will be tested against the corresponding value in the
    // instance.
    bool chosen(const EglConfig& dummy) const;

    // Return the EGL_SURFACE_TYPE value.
    EGLint surfaceType() const { return m_surface_type;};

    // Return the EGL_CONFIG_ID value.
    EGLint id() const { return m_config_id; }

    // Return the native configuration id. By default, this is the same as id()
    // Except if the EglConfig was created with the overriding copy-constructor
    // below. In which case nativeId() is the id() value of the source
    // EglConfig, while id() will return the overriden value.
    EGLint nativeId() const { return m_native_config_id; }

    // Return the native pixel format for this config.
    EglOS::PixelFormat* nativeFormat() const { return m_nativeFormat; }

    // Constructor for a new instance, all values explicitly defined.
    EglConfig(EGLint red_size,
              EGLint green_size,
              EGLint blue_size,
              EGLint alpha_size,
              EGLenum  caveat,
              EGLint config_id,
              EGLint depth_size,
              EGLint frame_buffer_level,
              EGLint max_pbuffer_width,
              EGLint max_pbuffer_height,
              EGLint max_pbuffer_size,
              EGLBoolean native_renderable,
              EGLint renderable_type,
              EGLint native_visual_id,
              EGLint native_visual_type,
              EGLint samples_per_pixel,
              EGLint stencil_size,
              EGLint surface_type,
              EGLenum transparent_type,
              EGLint trans_red_val,
              EGLint trans_green_val,
              EGLint trans_blue_val,
              EglOS::PixelFormat* frmt);

    // Copy-constructor.
    EglConfig(const EglConfig& conf);

    // A copy-constructor that allows one to override the configuration ID
    // and red/green/blue/alpha sizes. Note that this is how one creates
    // an EglConfig instance where nativeId() and id() return different
    // values (see comment for nativeId()).
    EglConfig(const EglConfig& conf,
              EGLint config_id,
              EGLint red_size,
              EGLint green_size,
              EGLint blue_size,
              EGLint alpha_size);

    // Destructor is required to get id of pixel format instance.
    ~EglConfig() {
        delete m_nativeFormat;
    }

private:
    const EGLint      m_buffer_size;
    const EGLint      m_red_size;
    const EGLint      m_green_size;
    const EGLint      m_blue_size;
    const EGLint      m_alpha_size;
    const EGLBoolean  m_bind_to_tex_rgb;
    const EGLBoolean  m_bind_to_tex_rgba;
    const EGLenum     m_caveat;
    const EGLint      m_config_id;
    const EGLint      m_native_config_id;
    const EGLint      m_frame_buffer_level;
    const EGLint      m_depth_size;
    const EGLint      m_max_pbuffer_width;
    const EGLint      m_max_pbuffer_height;
    const EGLint      m_max_pbuffer_size;
    const EGLint      m_max_swap_interval;
    const EGLint      m_min_swap_interval;
    const EGLBoolean  m_native_renderable;
    const EGLint      m_renderable_type;
    const EGLint      m_native_visual_id;
    const EGLint      m_native_visual_type;
    const EGLint      m_sample_buffers_num;
    const EGLint      m_samples_per_pixel;
    const EGLint      m_stencil_size;
    const EGLint      m_surface_type;
    const EGLenum     m_transparent_type;
    const EGLint      m_trans_red_val;
    const EGLint      m_trans_green_val;
    const EGLint      m_trans_blue_val;
    const EGLenum     m_conformant;

    EglOS::PixelFormat*  m_nativeFormat;
};

typedef std::list<EglConfig*> ConfigsList;

#endif
