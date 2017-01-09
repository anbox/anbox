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
#ifndef _EGL_PROC_H
#define _EGL_PROC_H

#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>

typedef EGLint (* eglGetError_t) ();
typedef EGLDisplay (* eglGetDisplay_t) (EGLNativeDisplayType);
typedef EGLBoolean (* eglInitialize_t) (EGLDisplay, EGLint*, EGLint*);
typedef EGLBoolean (* eglTerminate_t) (EGLDisplay);
typedef char* (* eglQueryString_t) (EGLDisplay, EGLint);
typedef EGLBoolean (* eglGetConfigs_t) (EGLDisplay, EGLConfig*, EGLint, EGLint*);
typedef EGLBoolean (* eglChooseConfig_t) (EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*);
typedef EGLBoolean (* eglGetConfigAttrib_t) (EGLDisplay, EGLConfig, EGLint, EGLint*);
typedef EGLSurface (* eglCreateWindowSurface_t) (EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint*);
typedef EGLSurface (* eglCreatePbufferSurface_t) (EGLDisplay, EGLConfig, const EGLint*);
typedef EGLSurface (* eglCreatePixmapSurface_t) (EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint*);
typedef EGLBoolean (* eglDestroySurface_t) (EGLDisplay, EGLSurface);
typedef EGLBoolean (* eglQuerySurface_t) (EGLDisplay, EGLSurface, EGLint, EGLint*);
typedef EGLBoolean (* eglBindAPI_t) (EGLenum);
typedef EGLenum (* eglQueryAPI_t) ();
typedef EGLBoolean (* eglWaitClient_t) ();
typedef EGLBoolean (* eglReleaseThread_t) ();
typedef EGLSurface (* eglCreatePbufferFromClientBuffer_t) (EGLDisplay, EGLenum, EGLClientBuffer, EGLConfig, const EGLint*);
typedef EGLBoolean (* eglSurfaceAttrib_t) (EGLDisplay, EGLSurface, EGLint, EGLint);
typedef EGLBoolean (* eglBindTexImage_t) (EGLDisplay, EGLSurface, EGLint);
typedef EGLBoolean (* eglReleaseTexImage_t) (EGLDisplay, EGLSurface, EGLint);
typedef EGLBoolean (* eglSwapInterval_t) (EGLDisplay, EGLint);
typedef EGLContext (* eglCreateContext_t) (EGLDisplay, EGLConfig, EGLContext, const EGLint*);
typedef EGLBoolean (* eglDestroyContext_t) (EGLDisplay, EGLContext);
typedef EGLBoolean (* eglMakeCurrent_t) (EGLDisplay, EGLSurface, EGLSurface, EGLContext);
typedef EGLContext (* eglGetCurrentContext_t) ();
typedef EGLSurface (* eglGetCurrentSurface_t) (EGLint);
typedef EGLDisplay (* eglGetCurrentDisplay_t) ();
typedef EGLBoolean (* eglQueryContext_t) (EGLDisplay, EGLContext, EGLint, EGLint*);
typedef EGLBoolean (* eglWaitGL_t) ();
typedef EGLBoolean (* eglWaitNative_t) (EGLint);
typedef EGLBoolean (* eglSwapBuffers_t) (EGLDisplay, EGLSurface);
typedef EGLBoolean (* eglCopyBuffers_t) (EGLDisplay, EGLSurface, EGLNativePixmapType);
typedef __eglMustCastToProperFunctionPointerType (* eglGetProcAddress_t) (const char*);
typedef EGLBoolean (* eglLockSurfaceKHR_t) (EGLDisplay, EGLSurface, const EGLint*);
typedef EGLBoolean (* eglUnlockSurfaceKHR_t) (EGLDisplay, EGLSurface);
typedef EGLImageKHR (* eglCreateImageKHR_t) (EGLDisplay, EGLContext, EGLenum, EGLClientBuffer, const EGLint*);
typedef EGLBoolean (* eglDestroyImageKHR_t) (EGLDisplay, EGLImageKHR image);
typedef EGLSyncKHR (* eglCreateSyncKHR_t) (EGLDisplay, EGLenum, const EGLint*);
typedef EGLBoolean (* eglDestroySyncKHR_t) (EGLDisplay, EGLSyncKHR sync);
typedef EGLint (* eglClientWaitSyncKHR_t) (EGLDisplay, EGLSyncKHR, EGLint, EGLTimeKHR timeout);
typedef EGLBoolean (* eglSignalSyncKHR_t) (EGLDisplay, EGLSyncKHR, EGLenum);
typedef EGLBoolean (* eglGetSyncAttribKHR_t) (EGLDisplay, EGLSyncKHR, EGLint, EGLint*);
typedef EGLBoolean (* eglSetSwapRectangleANDROID_t) (EGLDisplay, EGLSurface, EGLint, EGLint, EGLint, EGLint);

#endif // of  _EGL_PROC_H
