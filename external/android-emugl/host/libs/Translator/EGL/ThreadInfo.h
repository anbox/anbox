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
#ifndef THREAD_INFO_H
#define THREAD_INFO_H

#include "EglContext.h"

class EglDisplay;
class GLEScontext;

struct ThreadInfo {
    ThreadInfo():eglDisplay(NULL),glesContext(NULL),objManager(NULL){}

    void updateInfo(ContextPtr eglctx,
                    EglDisplay* dpy,
                    GLEScontext* glesCtx,
                    ShareGroupPtr share,
                    ObjectNameManager* manager);

    ContextPtr           eglContext;
    EglDisplay*          eglDisplay;
    GLEScontext*         glesContext;
    ShareGroupPtr        shareGroup;
    ObjectNameManager*   objManager;
};

ThreadInfo* getThreadInfo();

#endif
