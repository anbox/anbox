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
#include "EglPbufferSurface.h"

bool EglPbufferSurface::setAttrib(EGLint attrib,EGLint val) {
    switch(attrib) {
    case EGL_WIDTH:
        if(val < 0) return false;
        m_width = val;
        break;
    case EGL_HEIGHT:
        if(val < 0) return false;
        m_height = val;
        break;
    case EGL_LARGEST_PBUFFER:
        m_largest = val;
        break;
    case EGL_TEXTURE_FORMAT:
        if(val != EGL_NO_TEXTURE && val != EGL_TEXTURE_RGB && val != EGL_TEXTURE_RGBA) return false;
        m_texFormat = val;
        break;
    case EGL_TEXTURE_TARGET:
        if(val != EGL_NO_TEXTURE && val != EGL_TEXTURE_2D) return false;
        m_texTarget = val;
        break;
    case EGL_MIPMAP_TEXTURE:
        m_texMipmap = val;
        break;
    default:
        return false;
    }
    return true;
}

bool EglPbufferSurface::getAttrib(EGLint attrib,EGLint* val) {
    switch(attrib) {
    case EGL_CONFIG_ID:
        *val = m_config->id();
        break;
    case EGL_WIDTH:
        *val = m_width;
        break;
    case EGL_HEIGHT:
        *val = m_height;
        break;
    case EGL_LARGEST_PBUFFER:
        *val = m_largest;
        break;
    case EGL_TEXTURE_FORMAT:
        *val = m_texFormat;
        break;
    case EGL_TEXTURE_TARGET:
        *val = m_texTarget;
        break;
    case EGL_MIPMAP_TEXTURE:
        *val = m_texMipmap;
        break;
    default:
        return false;
    }
    return true;
}
