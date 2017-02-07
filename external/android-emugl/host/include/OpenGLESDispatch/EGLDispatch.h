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
#pragma once

#include "OpenGLESDispatch/RenderEGL_functions.h"
#include "OpenGLESDispatch/RenderEGL_extensions_functions.h"

// This header is used to define the EGLDispatch structure that contains
// pointers to the EGL shared library used by libOpenglRender. Normally,
// this will be our own libEGL_translator, but one could imagine a
// vendor-specific being used instead.

// There is a single global instance of this structure, named |s_egl|,
// which must be initialized by calling init_egl_dispatch() before use.

// Note that our code requires the implementation of misc EGL extensions
// including eglSetSwapRectangleANDROID(), see RenderEGL_extensions_functions.h
// for a full list.

#define RENDER_EGL_DEFINE_TYPE(return_type, function_name, signature) \
    typedef return_type (EGLAPIENTRY *function_name ## _t) signature;

#define RENDER_EGL_DECLARE_MEMBER(return_type, function_name, signature) \
    function_name ## _t function_name;

// Define function typedefs.
LIST_RENDER_EGL_FUNCTIONS(RENDER_EGL_DEFINE_TYPE)
LIST_RENDER_EGL_EXTENSIONS_FUNCTIONS(RENDER_EGL_DEFINE_TYPE)

// Define EGLDispatch structure.
struct EGLDispatch {
    bool initialized = false;
    LIST_RENDER_EGL_FUNCTIONS(RENDER_EGL_DECLARE_MEMBER)
    LIST_RENDER_EGL_EXTENSIONS_FUNCTIONS(RENDER_EGL_DECLARE_MEMBER)
};

// Initialize EGLDispatch function. Return true on success, false on failure.
bool init_egl_dispatch(const char *path);

// Global EGLDispatch instance. Call init_egl_dispatch() before using it.
extern EGLDispatch s_egl;
