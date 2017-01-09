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
#ifndef _EGL_CONTEXT_H
#define _EGL_CONTEXT_H

#include "GLClientState.h"
#include "GLSharedGroup.h"

struct EGLContext_t {

    enum {
        IS_CURRENT      =   0x00010000,
        NEVER_CURRENT   =   0x00020000
    };

    EGLContext_t(EGLDisplay dpy, EGLConfig config, EGLContext_t* shareCtx);
    ~EGLContext_t();
    uint32_t            flags;
    EGLDisplay          dpy;
    EGLConfig           config;
    EGLSurface          read;
    EGLSurface          draw;
    EGLContext_t    *   shareCtx;
    EGLint                version;
    uint32_t             rcContext;
    const char*         versionString;
    const char*         vendorString;
    const char*         rendererString;
    const char*         shaderVersionString;
    const char*         extensionString;
    EGLint              deletePending;
    GLClientState * getClientState(){ return clientState; }
    GLSharedGroupPtr getSharedGroup(){ return sharedGroup; }
private:
    GLClientState    *    clientState;
    GLSharedGroupPtr      sharedGroup;
};

#endif
