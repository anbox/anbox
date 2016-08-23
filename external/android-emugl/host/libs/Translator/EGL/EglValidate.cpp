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
#include "EglValidate.h"
#include <GLcommon/GLutils.h>

// static
bool EglValidate::confAttrib(EGLint attrib) {
    switch(attrib) {
    case EGL_BUFFER_SIZE:
    case EGL_RED_SIZE:
    case EGL_GREEN_SIZE:
    case EGL_BLUE_SIZE:
    case EGL_ALPHA_SIZE:
    case EGL_BIND_TO_TEXTURE_RGB:
    case EGL_BIND_TO_TEXTURE_RGBA:
    case EGL_CONFIG_CAVEAT:
    case EGL_CONFIG_ID:
    case EGL_DEPTH_SIZE:
    case EGL_LEVEL:
    case EGL_MAX_PBUFFER_WIDTH:
    case EGL_MAX_PBUFFER_HEIGHT:
    case EGL_MAX_PBUFFER_PIXELS:
    case EGL_MAX_SWAP_INTERVAL:
    case EGL_MIN_SWAP_INTERVAL:
    case EGL_RENDERABLE_TYPE:
    case EGL_NATIVE_RENDERABLE:
    case EGL_NATIVE_VISUAL_ID:
    case EGL_NATIVE_VISUAL_TYPE:
    case EGL_SAMPLE_BUFFERS:
    case EGL_SAMPLES:
    case EGL_STENCIL_SIZE:
    case EGL_SURFACE_TYPE:
    case EGL_TRANSPARENT_TYPE:
    case EGL_TRANSPARENT_RED_VALUE:
    case EGL_TRANSPARENT_GREEN_VALUE:
    case EGL_TRANSPARENT_BLUE_VALUE:
    case EGL_CONFORMANT:
        return true;
    }
    return false;
}

// static
bool EglValidate::noAttribs(const EGLint* attrib) {
    return !attrib || attrib[0] == EGL_NONE ;
}

// static
bool EglValidate::pbufferAttribs(EGLint width,
                                 EGLint height,
                                 bool isTexFormatNoTex,
                                 bool isTexTargetNoTex) {
    if(!isTexFormatNoTex) {
        if (!(isPowerOf2(width) && isPowerOf2(height))) {
            return false;
        }
    }
    return isTexFormatNoTex == isTexTargetNoTex ;
}

// static
bool EglValidate::releaseContext(EGLContext ctx,
                                 EGLSurface s1,
                                 EGLSurface s2) {
    return (ctx == EGL_NO_CONTEXT) &&
           (s1 == EGL_NO_SURFACE)  &&
           (s2 == EGL_NO_SURFACE);
}

// static
bool EglValidate::badContextMatch(EGLContext ctx,
                                  EGLSurface s1,
                                  EGLSurface s2) {
    return (ctx != EGL_NO_CONTEXT)
            ? (s1 == EGL_NO_SURFACE || s2 == EGL_NO_SURFACE)
            : (s1 != EGL_NO_SURFACE || s2 != EGL_NO_SURFACE);
}

// static
bool EglValidate::surfaceTarget(EGLint target) {
    return target == EGL_READ || target == EGL_DRAW;
}

// static
bool EglValidate::engine(EGLint engine) {
    return engine == EGL_CORE_NATIVE_ENGINE;
}

// static
bool EglValidate::stringName(EGLint name) {
    return name == EGL_VENDOR  ||
           name == EGL_VERSION ||
           name == EGL_EXTENSIONS;
}

// static
bool EglValidate::supportedApi(EGLenum api) {
    return api == EGL_OPENGL_ES_API;
}
