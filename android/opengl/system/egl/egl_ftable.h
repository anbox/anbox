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
static const struct _egl_funcs_by_name {
    const char *name;
    void *proc;
} egl_funcs_by_name[] = {
    {"eglGetError", (void *)eglGetError},
    {"eglGetDisplay", (void *)eglGetDisplay},
    {"eglInitialize", (void *)eglInitialize},
    {"eglTerminate", (void *)eglTerminate},
    {"eglQueryString", (void *)eglQueryString},
    {"eglGetConfigs", (void *)eglGetConfigs},
    {"eglChooseConfig", (void *)eglChooseConfig},
    {"eglGetConfigAttrib", (void *)eglGetConfigAttrib},
    {"eglCreateWindowSurface", (void *)eglCreateWindowSurface},
    {"eglCreatePbufferSurface", (void *)eglCreatePbufferSurface},
    {"eglCreatePixmapSurface", (void *)eglCreatePixmapSurface},
    {"eglDestroySurface", (void *)eglDestroySurface},
    {"eglQuerySurface", (void *)eglQuerySurface},
    {"eglBindAPI", (void *)eglBindAPI},
    {"eglQueryAPI", (void *)eglQueryAPI},
    {"eglWaitClient", (void *)eglWaitClient},
    {"eglReleaseThread", (void *)eglReleaseThread},
    {"eglCreatePbufferFromClientBuffer", (void *)eglCreatePbufferFromClientBuffer},
    {"eglSurfaceAttrib", (void *)eglSurfaceAttrib},
    {"eglBindTexImage", (void *)eglBindTexImage},
    {"eglReleaseTexImage", (void *)eglReleaseTexImage},
    {"eglSwapInterval", (void *)eglSwapInterval},
    {"eglCreateContext", (void *)eglCreateContext},
    {"eglDestroyContext", (void *)eglDestroyContext},
    {"eglMakeCurrent", (void *)eglMakeCurrent},
    {"eglGetCurrentContext", (void *)eglGetCurrentContext},
    {"eglGetCurrentSurface", (void *)eglGetCurrentSurface},
    {"eglGetCurrentDisplay", (void *)eglGetCurrentDisplay},
    {"eglQueryContext", (void *)eglQueryContext},
    {"eglWaitGL", (void *)eglWaitGL},
    {"eglWaitNative", (void *)eglWaitNative},
    {"eglSwapBuffers", (void *)eglSwapBuffers},
    {"eglCopyBuffers", (void *)eglCopyBuffers},
    {"eglGetProcAddress", (void *)eglGetProcAddress},
    {"eglLockSurfaceKHR", (void *)eglLockSurfaceKHR},
    {"eglUnlockSurfaceKHR", (void *)eglUnlockSurfaceKHR},
    {"eglCreateImageKHR", (void *)eglCreateImageKHR},
    {"eglDestroyImageKHR", (void *)eglDestroyImageKHR},
    {"eglCreateSyncKHR", (void *)eglCreateSyncKHR},
    {"eglDestroySyncKHR", (void *)eglDestroySyncKHR},
    {"eglClientWaitSyncKHR", (void *)eglClientWaitSyncKHR},
    {"eglGetSyncAttribKHR", (void *)eglGetSyncAttribKHR}
};

static const int egl_num_funcs = sizeof(egl_funcs_by_name) / sizeof(struct _egl_funcs_by_name);
