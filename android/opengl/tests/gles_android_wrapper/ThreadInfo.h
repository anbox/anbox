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
#ifndef _THREAD_INFO_H
#define _THREAD_INFO_H

#include "ServerConnection.h"
#include <EGL/egl.h>

struct EGLWrapperContext
{
    EGLWrapperContext(EGLContext p_aglContext, int _version) {
        aglContext = p_aglContext;
        clientState = NULL;
        version = _version;
    }

    ~EGLWrapperContext() {
        delete clientState;
    }

    EGLContext aglContext;
    GLClientState *clientState;
    int version;
};

struct EGLThreadInfo
{
    EGLThreadInfo() : currentContext(NULL), serverConn(NULL) {}

    EGLWrapperContext *currentContext;
    ServerConnection *serverConn;
};


EGLThreadInfo *getEGLThreadInfo();
#endif
