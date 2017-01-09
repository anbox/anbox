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
#ifndef _EGL_DISPATCH_H
#define _EGL_DISPATCH_H

#include "egl_proc.h"

struct egl_dispatch {
    eglGetError_t eglGetError;
    eglGetDisplay_t eglGetDisplay;
    eglInitialize_t eglInitialize;
    eglTerminate_t eglTerminate;
    eglQueryString_t eglQueryString;
    eglGetConfigs_t eglGetConfigs;
    eglChooseConfig_t eglChooseConfig;
    eglGetConfigAttrib_t eglGetConfigAttrib;
    eglCreateWindowSurface_t eglCreateWindowSurface;
    eglCreatePbufferSurface_t eglCreatePbufferSurface;
    eglCreatePixmapSurface_t eglCreatePixmapSurface;
    eglDestroySurface_t eglDestroySurface;
    eglQuerySurface_t eglQuerySurface;
    eglBindAPI_t eglBindAPI;
    eglQueryAPI_t eglQueryAPI;
    eglWaitClient_t eglWaitClient;
    eglReleaseThread_t eglReleaseThread;
    eglCreatePbufferFromClientBuffer_t eglCreatePbufferFromClientBuffer;
    eglSurfaceAttrib_t eglSurfaceAttrib;
    eglBindTexImage_t eglBindTexImage;
    eglReleaseTexImage_t eglReleaseTexImage;
    eglSwapInterval_t eglSwapInterval;
    eglCreateContext_t eglCreateContext;
    eglDestroyContext_t eglDestroyContext;
    eglMakeCurrent_t eglMakeCurrent;
    eglGetCurrentContext_t eglGetCurrentContext;
    eglGetCurrentSurface_t eglGetCurrentSurface;
    eglGetCurrentDisplay_t eglGetCurrentDisplay;
    eglQueryContext_t eglQueryContext;
    eglWaitGL_t eglWaitGL;
    eglWaitNative_t eglWaitNative;
    eglSwapBuffers_t eglSwapBuffers;
    eglCopyBuffers_t eglCopyBuffers;
    eglGetProcAddress_t eglGetProcAddress;
    eglLockSurfaceKHR_t eglLockSurfaceKHR;
    eglUnlockSurfaceKHR_t eglUnlockSurfaceKHR;
    eglCreateImageKHR_t eglCreateImageKHR;
    eglDestroyImageKHR_t eglDestroyImageKHR;
    eglCreateSyncKHR_t eglCreateSyncKHR;
    eglDestroySyncKHR_t eglDestroySyncKHR;
    eglClientWaitSyncKHR_t eglClientWaitSyncKHR;
    eglSignalSyncKHR_t eglSignalSyncKHR;
    eglGetSyncAttribKHR_t eglGetSyncAttribKHR;
    eglSetSwapRectangleANDROID_t eglSetSwapRectangleANDROID;
    //Accessors
    eglGetError_t set_eglGetError(eglGetError_t f) { eglGetError_t retval = eglGetError; eglGetError = f; return retval;}
    eglGetDisplay_t set_eglGetDisplay(eglGetDisplay_t f) { eglGetDisplay_t retval = eglGetDisplay; eglGetDisplay = f; return retval;}
    eglInitialize_t set_eglInitialize(eglInitialize_t f) { eglInitialize_t retval = eglInitialize; eglInitialize = f; return retval;}
    eglTerminate_t set_eglTerminate(eglTerminate_t f) { eglTerminate_t retval = eglTerminate; eglTerminate = f; return retval;}
    eglQueryString_t set_eglQueryString(eglQueryString_t f) { eglQueryString_t retval = eglQueryString; eglQueryString = f; return retval;}
    eglGetConfigs_t set_eglGetConfigs(eglGetConfigs_t f) { eglGetConfigs_t retval = eglGetConfigs; eglGetConfigs = f; return retval;}
    eglChooseConfig_t set_eglChooseConfig(eglChooseConfig_t f) { eglChooseConfig_t retval = eglChooseConfig; eglChooseConfig = f; return retval;}
    eglGetConfigAttrib_t set_eglGetConfigAttrib(eglGetConfigAttrib_t f) { eglGetConfigAttrib_t retval = eglGetConfigAttrib; eglGetConfigAttrib = f; return retval;}
    eglCreateWindowSurface_t set_eglCreateWindowSurface(eglCreateWindowSurface_t f) { eglCreateWindowSurface_t retval = eglCreateWindowSurface; eglCreateWindowSurface = f; return retval;}
    eglCreatePbufferSurface_t set_eglCreatePbufferSurface(eglCreatePbufferSurface_t f) { eglCreatePbufferSurface_t retval = eglCreatePbufferSurface; eglCreatePbufferSurface = f; return retval;}
    eglCreatePixmapSurface_t set_eglCreatePixmapSurface(eglCreatePixmapSurface_t f) { eglCreatePixmapSurface_t retval = eglCreatePixmapSurface; eglCreatePixmapSurface = f; return retval;}
    eglDestroySurface_t set_eglDestroySurface(eglDestroySurface_t f) { eglDestroySurface_t retval = eglDestroySurface; eglDestroySurface = f; return retval;}
    eglQuerySurface_t set_eglQuerySurface(eglQuerySurface_t f) { eglQuerySurface_t retval = eglQuerySurface; eglQuerySurface = f; return retval;}
    eglBindAPI_t set_eglBindAPI(eglBindAPI_t f) { eglBindAPI_t retval = eglBindAPI; eglBindAPI = f; return retval;}
    eglQueryAPI_t set_eglQueryAPI(eglQueryAPI_t f) { eglQueryAPI_t retval = eglQueryAPI; eglQueryAPI = f; return retval;}
    eglWaitClient_t set_eglWaitClient(eglWaitClient_t f) { eglWaitClient_t retval = eglWaitClient; eglWaitClient = f; return retval;}
    eglReleaseThread_t set_eglReleaseThread(eglReleaseThread_t f) { eglReleaseThread_t retval = eglReleaseThread; eglReleaseThread = f; return retval;}
    eglCreatePbufferFromClientBuffer_t set_eglCreatePbufferFromClientBuffer(eglCreatePbufferFromClientBuffer_t f) { eglCreatePbufferFromClientBuffer_t retval = eglCreatePbufferFromClientBuffer; eglCreatePbufferFromClientBuffer = f; return retval;}
    eglSurfaceAttrib_t set_eglSurfaceAttrib(eglSurfaceAttrib_t f) { eglSurfaceAttrib_t retval = eglSurfaceAttrib; eglSurfaceAttrib = f; return retval;}
    eglBindTexImage_t set_eglBindTexImage(eglBindTexImage_t f) { eglBindTexImage_t retval = eglBindTexImage; eglBindTexImage = f; return retval;}
    eglReleaseTexImage_t set_eglReleaseTexImage(eglReleaseTexImage_t f) { eglReleaseTexImage_t retval = eglReleaseTexImage; eglReleaseTexImage = f; return retval;}
    eglSwapInterval_t set_eglSwapInterval(eglSwapInterval_t f) { eglSwapInterval_t retval = eglSwapInterval; eglSwapInterval = f; return retval;}
    eglCreateContext_t set_eglCreateContext(eglCreateContext_t f) { eglCreateContext_t retval = eglCreateContext; eglCreateContext = f; return retval;}
    eglDestroyContext_t set_eglDestroyContext(eglDestroyContext_t f) { eglDestroyContext_t retval = eglDestroyContext; eglDestroyContext = f; return retval;}
    eglMakeCurrent_t set_eglMakeCurrent(eglMakeCurrent_t f) { eglMakeCurrent_t retval = eglMakeCurrent; eglMakeCurrent = f; return retval;}
    eglGetCurrentContext_t set_eglGetCurrentContext(eglGetCurrentContext_t f) { eglGetCurrentContext_t retval = eglGetCurrentContext; eglGetCurrentContext = f; return retval;}
    eglGetCurrentSurface_t set_eglGetCurrentSurface(eglGetCurrentSurface_t f) { eglGetCurrentSurface_t retval = eglGetCurrentSurface; eglGetCurrentSurface = f; return retval;}
    eglGetCurrentDisplay_t set_eglGetCurrentDisplay(eglGetCurrentDisplay_t f) { eglGetCurrentDisplay_t retval = eglGetCurrentDisplay; eglGetCurrentDisplay = f; return retval;}
    eglQueryContext_t set_eglQueryContext(eglQueryContext_t f) { eglQueryContext_t retval = eglQueryContext; eglQueryContext = f; return retval;}
    eglWaitGL_t set_eglWaitGL(eglWaitGL_t f) { eglWaitGL_t retval = eglWaitGL; eglWaitGL = f; return retval;}
    eglWaitNative_t set_eglWaitNative(eglWaitNative_t f) { eglWaitNative_t retval = eglWaitNative; eglWaitNative = f; return retval;}
    eglSwapBuffers_t set_eglSwapBuffers(eglSwapBuffers_t f) { eglSwapBuffers_t retval = eglSwapBuffers; eglSwapBuffers = f; return retval;}
    eglCopyBuffers_t set_eglCopyBuffers(eglCopyBuffers_t f) { eglCopyBuffers_t retval = eglCopyBuffers; eglCopyBuffers = f; return retval;}
    eglGetProcAddress_t set_eglGetProcAddress(eglGetProcAddress_t f) { eglGetProcAddress_t retval = eglGetProcAddress; eglGetProcAddress = f; return retval;}
    eglLockSurfaceKHR_t set_eglLockSurfaceKHR(eglLockSurfaceKHR_t f) { eglLockSurfaceKHR_t retval = eglLockSurfaceKHR; eglLockSurfaceKHR = f; return retval;}
    eglUnlockSurfaceKHR_t set_eglUnlockSurfaceKHR(eglUnlockSurfaceKHR_t f) { eglUnlockSurfaceKHR_t retval = eglUnlockSurfaceKHR; eglUnlockSurfaceKHR = f; return retval;}
    eglCreateImageKHR_t set_eglCreateImageKHR(eglCreateImageKHR_t f) { eglCreateImageKHR_t retval = eglCreateImageKHR; eglCreateImageKHR = f; return retval;}
    eglDestroyImageKHR_t set_eglDestroyImageKHR(eglDestroyImageKHR_t f) { eglDestroyImageKHR_t retval = eglDestroyImageKHR; eglDestroyImageKHR = f; return retval;}
    eglCreateSyncKHR_t set_eglCreateSyncKHR(eglCreateSyncKHR_t f) { eglCreateSyncKHR_t retval = eglCreateSyncKHR; eglCreateSyncKHR = f; return retval;}
    eglDestroySyncKHR_t set_eglDestroySyncKHR(eglDestroySyncKHR_t f) { eglDestroySyncKHR_t retval = eglDestroySyncKHR; eglDestroySyncKHR = f; return retval;}
    eglClientWaitSyncKHR_t set_eglClientWaitSyncKHR(eglClientWaitSyncKHR_t f) { eglClientWaitSyncKHR_t retval = eglClientWaitSyncKHR; eglClientWaitSyncKHR = f; return retval;}
    eglSignalSyncKHR_t set_eglSignalSyncKHR(eglSignalSyncKHR_t f) { eglSignalSyncKHR_t retval = eglSignalSyncKHR; eglSignalSyncKHR = f; return retval;}
    eglGetSyncAttribKHR_t set_eglGetSyncAttribKHR(eglGetSyncAttribKHR_t f) { eglGetSyncAttribKHR_t retval = eglGetSyncAttribKHR; eglGetSyncAttribKHR = f; return retval;}
    eglSetSwapRectangleANDROID_t set_eglSetSwapRectangleANDROID(eglSetSwapRectangleANDROID_t f) { eglSetSwapRectangleANDROID_t retval = eglSetSwapRectangleANDROID; eglSetSwapRectangleANDROID = f; return retval;}
};

egl_dispatch *create_egl_dispatch(void *gles_andorid);

#endif
