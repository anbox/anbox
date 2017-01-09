/*
* Copyright 2011 The Android Open Source Project
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

#ifndef _SYSTEM_COMMON_EGL_CLIENT_IFACE_H
#define _SYSTEM_COMMON_EGL_CLIENT_IFACE_H

struct EGLThreadInfo;  // defined in ThreadInfo.h

typedef struct {
    EGLThreadInfo* (*getThreadInfo)();
    const char* (*getGLString)(int glEnum);
} EGLClient_eglInterface;

typedef struct {
    void* (*getProcAddress)(const char *funcName);
    void (*init)();
    void (*finish)();
} EGLClient_glesInterface;

//
// Any GLES/GLES2 client API library should define a function named "init_emul_gles"
// with the following prototype,
// It will be called by EGL after loading the GLES library for initialization
// and exchanging interface function pointers.
//
typedef EGLClient_glesInterface *(*init_emul_gles_t)(EGLClient_eglInterface *eglIface);

#endif
