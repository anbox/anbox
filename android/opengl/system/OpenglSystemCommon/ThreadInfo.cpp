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
#include "ThreadInfo.h"
#include "cutils/threads.h"

thread_store_t s_tls = THREAD_STORE_INITIALIZER;

static void tlsDestruct(void *ptr)
{
    if (ptr) {
        EGLThreadInfo *ti = (EGLThreadInfo *)ptr;
        delete ti->hostConn;
        delete ti;
        ((void **)__get_tls())[TLS_SLOT_OPENGL] = NULL;
    }
}

EGLThreadInfo *slow_getEGLThreadInfo()
{
    EGLThreadInfo *ti = (EGLThreadInfo *)thread_store_get(&s_tls);
    if (ti) return ti;

    ti = new EGLThreadInfo();
    thread_store_set(&s_tls, ti, tlsDestruct);

    return ti;
}
