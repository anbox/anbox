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
#include "EglConfig.h"

EglConfig::EglConfig(EGLint     red_size,
                     EGLint     green_size,
                     EGLint     blue_size,
                     EGLint     alpha_size,
                     EGLenum    caveat,
                     EGLint     config_id,
                     EGLint     depth_size,
                     EGLint     frame_buffer_level,
                     EGLint     max_pbuffer_width,
                     EGLint     max_pbuffer_height,
                     EGLint     max_pbuffer_size,
                     EGLBoolean native_renderable,
                     EGLint     renderable_type,
                     EGLint     native_visual_id,
                     EGLint     native_visual_type,
                     EGLint     samples_per_pixel,
                     EGLint     stencil_size,
                     EGLint     surface_type,
                     EGLenum    transparent_type,
                     EGLint     trans_red_val,
                     EGLint     trans_green_val,
                     EGLint     trans_blue_val,
                     EglOS::PixelFormat* frmt):
        m_buffer_size(red_size + green_size + blue_size + alpha_size),
        m_red_size(red_size),
        m_green_size(green_size),
        m_blue_size(blue_size),
        m_alpha_size(alpha_size),
        m_bind_to_tex_rgb(EGL_FALSE), //not supported for now
        m_bind_to_tex_rgba(EGL_FALSE), //not supported for now
        m_caveat(caveat),
        m_config_id(config_id),
        m_native_config_id(config_id),
        m_frame_buffer_level(frame_buffer_level),
        m_depth_size(depth_size),
        m_max_pbuffer_width(max_pbuffer_width),
        m_max_pbuffer_height(max_pbuffer_height),
        m_max_pbuffer_size(max_pbuffer_size),
        m_max_swap_interval(MAX_SWAP_INTERVAL),
        m_min_swap_interval(MIN_SWAP_INTERVAL),
        m_native_renderable(native_renderable),
        m_renderable_type(renderable_type),
        m_native_visual_id(native_visual_id),
        m_native_visual_type(native_visual_type),
        m_sample_buffers_num(samples_per_pixel > 0 ? 1 : 0),
        m_samples_per_pixel(samples_per_pixel),
        m_stencil_size(stencil_size),
        m_surface_type(surface_type),
        m_transparent_type(transparent_type),
        m_trans_red_val(trans_red_val),
        m_trans_green_val(trans_green_val),
        m_trans_blue_val(trans_blue_val),
        m_conformant(((red_size + green_size + blue_size + alpha_size > 0)  &&
                     (caveat != EGL_NON_CONFORMANT_CONFIG)) ?
                     m_renderable_type : 0),
        m_nativeFormat(frmt) {}


EglConfig::EglConfig(const EglConfig& conf) :
        m_buffer_size(conf.m_buffer_size),
        m_red_size(conf.m_red_size),
        m_green_size(conf.m_green_size),
        m_blue_size(conf.m_blue_size),
        m_alpha_size(conf.m_alpha_size),
        m_bind_to_tex_rgb(conf.m_bind_to_tex_rgb),
        m_bind_to_tex_rgba(conf.m_bind_to_tex_rgba),
        m_caveat(conf.m_caveat),
        m_config_id(conf.m_config_id),
        m_native_config_id(conf.m_native_config_id),
        m_frame_buffer_level(conf.m_frame_buffer_level),
        m_depth_size(conf.m_depth_size),
        m_max_pbuffer_width(conf.m_max_pbuffer_width),
        m_max_pbuffer_height(conf.m_max_pbuffer_height),
        m_max_pbuffer_size(conf.m_max_pbuffer_size),
        m_max_swap_interval(conf.m_max_swap_interval),
        m_min_swap_interval(conf.m_min_swap_interval),
        m_native_renderable(conf.m_native_renderable),
        m_renderable_type(conf.m_renderable_type),
        m_native_visual_id(conf.m_native_visual_id),
        m_native_visual_type(conf.m_native_visual_type),
        m_sample_buffers_num(conf.m_sample_buffers_num),
        m_samples_per_pixel(conf.m_samples_per_pixel),
        m_stencil_size(conf.m_stencil_size),
        m_surface_type(conf.m_surface_type),
        m_transparent_type(conf.m_transparent_type),
        m_trans_red_val(conf.m_trans_red_val),
        m_trans_green_val(conf.m_trans_green_val),
        m_trans_blue_val(conf.m_trans_blue_val),
        m_conformant(conf.m_conformant),
        m_nativeFormat(conf.m_nativeFormat->clone()) {}


EglConfig::EglConfig(const EglConfig& conf,
                     EGLint config_id,
                     EGLint red_size,
                     EGLint green_size,
                     EGLint blue_size,
                     EGLint alpha_size) :
        m_buffer_size(red_size + green_size + blue_size + alpha_size),
        m_red_size(red_size),
        m_green_size(green_size),
        m_blue_size(blue_size),
        m_alpha_size(alpha_size),
        m_bind_to_tex_rgb(conf.m_bind_to_tex_rgb),
        m_bind_to_tex_rgba(conf.m_bind_to_tex_rgba),
        m_caveat(conf.m_caveat),
        m_config_id(config_id),
        m_native_config_id(conf.m_native_config_id),
        m_frame_buffer_level(conf.m_frame_buffer_level),
        m_depth_size(conf.m_depth_size),
        m_max_pbuffer_width(conf.m_max_pbuffer_width),
        m_max_pbuffer_height(conf.m_max_pbuffer_height),
        m_max_pbuffer_size(conf.m_max_pbuffer_size),
        m_max_swap_interval(conf.m_max_swap_interval),
        m_min_swap_interval(conf.m_min_swap_interval),
        m_native_renderable(conf.m_native_renderable),
        m_renderable_type(conf.m_renderable_type),
        m_native_visual_id(conf.m_native_visual_id),
        m_native_visual_type(conf.m_native_visual_type),
        m_sample_buffers_num(conf.m_sample_buffers_num),
        m_samples_per_pixel(conf.m_samples_per_pixel),
        m_stencil_size(conf.m_stencil_size),
        m_surface_type(conf.m_surface_type),
        m_transparent_type(conf.m_transparent_type),
        m_trans_red_val(conf.m_trans_red_val),
        m_trans_green_val(conf.m_trans_green_val),
        m_trans_blue_val(conf.m_trans_blue_val),
        m_conformant(conf.m_conformant),
        m_nativeFormat(conf.m_nativeFormat->clone()) {};


bool EglConfig::getConfAttrib(EGLint attrib,EGLint* val) const {
    switch(attrib) {
    case EGL_BUFFER_SIZE:
        *val = m_buffer_size;
        break;
    case EGL_RED_SIZE:
        *val = m_red_size;
        break;
    case EGL_GREEN_SIZE:
        *val = m_green_size;
        break;
    case EGL_BLUE_SIZE:
        *val = m_blue_size;
        break;
    case EGL_ALPHA_SIZE:
        *val = m_alpha_size;
        break;
    case EGL_BIND_TO_TEXTURE_RGB:
        *val = m_bind_to_tex_rgb;
        break;
    case EGL_BIND_TO_TEXTURE_RGBA:
        *val = m_bind_to_tex_rgba;
        break;
    case EGL_CONFIG_CAVEAT:
        *val = m_caveat;
        break;
    case EGL_CONFIG_ID:
        *val = m_config_id;
        break;
    case EGL_DEPTH_SIZE:
        *val = m_depth_size;
        break;
    case EGL_LEVEL:
        *val = m_frame_buffer_level;
        break;
    case EGL_MAX_PBUFFER_WIDTH:
        *val = m_max_pbuffer_width;
        break;
    case EGL_MAX_PBUFFER_HEIGHT:
        *val = m_max_pbuffer_height;
        break;
    case EGL_MAX_PBUFFER_PIXELS:
        *val = m_max_pbuffer_size;
        break;
    case EGL_MAX_SWAP_INTERVAL:
        *val = m_max_swap_interval;
        break;
    case EGL_MIN_SWAP_INTERVAL:
        *val = m_min_swap_interval;
        break;
    case EGL_NATIVE_RENDERABLE:
        *val = m_native_renderable;
        break;
    case EGL_NATIVE_VISUAL_ID:
        *val = m_native_visual_id;
        break;
    case EGL_NATIVE_VISUAL_TYPE:
        *val = m_native_visual_type;
        break;
    case EGL_RENDERABLE_TYPE:
        *val = m_renderable_type;
        break;
    case EGL_SAMPLE_BUFFERS:
        *val = m_sample_buffers_num;
        break;
    case EGL_SAMPLES:
        *val = m_samples_per_pixel;
        break;
    case EGL_STENCIL_SIZE:
        *val = m_stencil_size;
        break;
    case EGL_SURFACE_TYPE:
        *val = m_surface_type;
        break;
    case EGL_TRANSPARENT_TYPE:
        *val =m_transparent_type;
        break;
    case EGL_TRANSPARENT_RED_VALUE:
        *val = m_trans_red_val;
        break;
    case EGL_TRANSPARENT_GREEN_VALUE:
        *val = m_trans_green_val;
        break;
    case EGL_TRANSPARENT_BLUE_VALUE:
        *val = m_trans_blue_val;
        break;
    case EGL_CONFORMANT:
        *val = m_conformant;
        break;
    default:
        return false;
    }
    return true;
}

// checking compitabilty between *this configuration and another configuration
// the compitability is checked againsed red,green,blue,buffer stencil and depth sizes
bool EglConfig::compatibleWith(const EglConfig& conf) const {

    return m_buffer_size == conf.m_buffer_size &&
           m_red_size == conf.m_red_size &&
           m_green_size == conf.m_green_size &&
           m_blue_size == conf.m_blue_size &&
           m_depth_size == conf.m_depth_size &&
           m_stencil_size == conf.m_stencil_size;
}

//following the sorting EGLconfig as in spec
bool EglConfig::operator<(const EglConfig& conf) const {
    //0
    if(m_conformant != conf.m_conformant) {
       return m_conformant != 0; //We want the conformant ones first
    }
    //1
    if(m_caveat != conf.m_caveat) {
       return m_caveat < conf.m_caveat; // EGL_NONE < EGL_SLOW_CONFIG < EGL_NON_CONFORMANT_CONFIG
    }
    //2 TODO:

    //3
    if(m_buffer_size != conf.m_buffer_size) {
       return m_buffer_size < conf.m_buffer_size;
    }
    //4
    if(m_sample_buffers_num != conf.m_sample_buffers_num) {
       return m_sample_buffers_num < conf.m_sample_buffers_num;
    }
    //5
    if(m_samples_per_pixel != conf.m_samples_per_pixel) {
       return m_samples_per_pixel < conf.m_samples_per_pixel;
    }
    //6
    if(m_depth_size != conf.m_depth_size) {
       return m_depth_size < conf.m_depth_size;
    }
    //7
    if(m_stencil_size != conf.m_stencil_size) {
       return m_stencil_size < conf.m_stencil_size;
    }
    //8 implementation defined
    if(m_native_visual_type != conf.m_native_visual_type) {
       return m_native_visual_type < conf.m_native_visual_type;
    }
    //9
    return m_config_id < conf.m_config_id;
}

bool EglConfig::operator>=(const EglConfig& conf) const {
    return  !((*this) < conf);
}
#define CHECK_PROP(dummy,prop_name,op) \
                  if((dummy.prop_name != EGL_DONT_CARE) && (dummy.prop_name op prop_name)) return false;
#define CHECK_PROP_CAST(dummy,prop_name,op) \
                  if((((EGLint)dummy.prop_name) != EGL_DONT_CARE) && (dummy.prop_name op prop_name)) return false;
//checking if config stands for all the selection crateria of dummy as defined by EGL spec
bool EglConfig::chosen(const EglConfig& dummy) const {

   //atleast
   CHECK_PROP(dummy,m_buffer_size,>);
   CHECK_PROP(dummy,m_red_size,>);
   CHECK_PROP(dummy,m_green_size,>);
   CHECK_PROP(dummy,m_blue_size,>);
   CHECK_PROP(dummy,m_alpha_size,>);
   CHECK_PROP(dummy,m_depth_size,>);
   CHECK_PROP(dummy,m_stencil_size,>);
   CHECK_PROP(dummy,m_sample_buffers_num,>);
   CHECK_PROP(dummy,m_samples_per_pixel,>);

   //exact
   CHECK_PROP(dummy,m_frame_buffer_level,!=);
   CHECK_PROP(dummy,m_config_id,!=);
   CHECK_PROP(dummy,m_native_visual_type,!=);
   CHECK_PROP(dummy,m_max_swap_interval ,!=);
   CHECK_PROP(dummy,m_min_swap_interval ,!=);
   CHECK_PROP(dummy,m_trans_red_val    ,!=);
   CHECK_PROP(dummy,m_trans_green_val ,!=);
   CHECK_PROP(dummy,m_trans_blue_val  ,!=);
   //exact - when cast to EGLint is needed when comparing to EGL_DONT_CARE
   CHECK_PROP_CAST(dummy,m_bind_to_tex_rgb ,!=);
   CHECK_PROP_CAST(dummy,m_bind_to_tex_rgba,!=);
   CHECK_PROP_CAST(dummy,m_caveat,!=);
   CHECK_PROP_CAST(dummy,m_native_renderable ,!=);
   CHECK_PROP_CAST(dummy,m_transparent_type   ,!=);

   //mask
   if(dummy.m_surface_type != EGL_DONT_CARE &&
    ((dummy.m_surface_type & m_surface_type) != dummy.m_surface_type)) return false;

   if(dummy.m_conformant != (EGLenum)EGL_DONT_CARE &&
    ((dummy.m_conformant & m_conformant) != dummy.m_conformant)) return false;

   if(dummy.m_renderable_type != EGL_DONT_CARE &&
    ((dummy.m_renderable_type & m_renderable_type) != dummy.m_renderable_type)) return false;

   //passed all checks
   return true;
}
