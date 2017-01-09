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
#include <stdio.h>
#include "egl_dispatch.h"
#include <dlfcn.h>

egl_dispatch *create_egl_dispatch(void *gles_android)
{
        egl_dispatch *disp = new egl_dispatch;

    void *ptr;
    ptr = dlsym(gles_android,"eglGetError"); disp->set_eglGetError((eglGetError_t)ptr);
    ptr = dlsym(gles_android,"eglGetDisplay"); disp->set_eglGetDisplay((eglGetDisplay_t)ptr);
    ptr = dlsym(gles_android,"eglInitialize"); disp->set_eglInitialize((eglInitialize_t)ptr);
    ptr = dlsym(gles_android,"eglTerminate"); disp->set_eglTerminate((eglTerminate_t)ptr);
    ptr = dlsym(gles_android,"eglQueryString"); disp->set_eglQueryString((eglQueryString_t)ptr);
    ptr = dlsym(gles_android,"eglGetConfigs"); disp->set_eglGetConfigs((eglGetConfigs_t)ptr);
    ptr = dlsym(gles_android,"eglChooseConfig"); disp->set_eglChooseConfig((eglChooseConfig_t)ptr);
    ptr = dlsym(gles_android,"eglGetConfigAttrib"); disp->set_eglGetConfigAttrib((eglGetConfigAttrib_t)ptr);
    ptr = dlsym(gles_android,"eglCreateWindowSurface"); disp->set_eglCreateWindowSurface((eglCreateWindowSurface_t)ptr);
    ptr = dlsym(gles_android,"eglCreatePbufferSurface"); disp->set_eglCreatePbufferSurface((eglCreatePbufferSurface_t)ptr);
    ptr = dlsym(gles_android,"eglCreatePixmapSurface"); disp->set_eglCreatePixmapSurface((eglCreatePixmapSurface_t)ptr);
    ptr = dlsym(gles_android,"eglDestroySurface"); disp->set_eglDestroySurface((eglDestroySurface_t)ptr);
    ptr = dlsym(gles_android,"eglQuerySurface"); disp->set_eglQuerySurface((eglQuerySurface_t)ptr);
    ptr = dlsym(gles_android,"eglBindAPI"); disp->set_eglBindAPI((eglBindAPI_t)ptr);
    ptr = dlsym(gles_android,"eglQueryAPI"); disp->set_eglQueryAPI((eglQueryAPI_t)ptr);
    ptr = dlsym(gles_android,"eglWaitClient"); disp->set_eglWaitClient((eglWaitClient_t)ptr);
    ptr = dlsym(gles_android,"eglReleaseThread"); disp->set_eglReleaseThread((eglReleaseThread_t)ptr);
    ptr = dlsym(gles_android,"eglCreatePbufferFromClientBuffer"); disp->set_eglCreatePbufferFromClientBuffer((eglCreatePbufferFromClientBuffer_t)ptr);
    ptr = dlsym(gles_android,"eglSurfaceAttrib"); disp->set_eglSurfaceAttrib((eglSurfaceAttrib_t)ptr);
    ptr = dlsym(gles_android,"eglBindTexImage"); disp->set_eglBindTexImage((eglBindTexImage_t)ptr);
    ptr = dlsym(gles_android,"eglReleaseTexImage"); disp->set_eglReleaseTexImage((eglReleaseTexImage_t)ptr);
    ptr = dlsym(gles_android,"eglSwapInterval"); disp->set_eglSwapInterval((eglSwapInterval_t)ptr);
    ptr = dlsym(gles_android,"eglCreateContext"); disp->set_eglCreateContext((eglCreateContext_t)ptr);
    ptr = dlsym(gles_android,"eglDestroyContext"); disp->set_eglDestroyContext((eglDestroyContext_t)ptr);
    ptr = dlsym(gles_android,"eglMakeCurrent"); disp->set_eglMakeCurrent((eglMakeCurrent_t)ptr);
    ptr = dlsym(gles_android,"eglGetCurrentContext"); disp->set_eglGetCurrentContext((eglGetCurrentContext_t)ptr);
    ptr = dlsym(gles_android,"eglGetCurrentSurface"); disp->set_eglGetCurrentSurface((eglGetCurrentSurface_t)ptr);
    ptr = dlsym(gles_android,"eglGetCurrentDisplay"); disp->set_eglGetCurrentDisplay((eglGetCurrentDisplay_t)ptr);
    ptr = dlsym(gles_android,"eglQueryContext"); disp->set_eglQueryContext((eglQueryContext_t)ptr);
    ptr = dlsym(gles_android,"eglWaitGL"); disp->set_eglWaitGL((eglWaitGL_t)ptr);
    ptr = dlsym(gles_android,"eglWaitNative"); disp->set_eglWaitNative((eglWaitNative_t)ptr);
    ptr = dlsym(gles_android,"eglSwapBuffers"); disp->set_eglSwapBuffers((eglSwapBuffers_t)ptr);
    ptr = dlsym(gles_android,"eglCopyBuffers"); disp->set_eglCopyBuffers((eglCopyBuffers_t)ptr);
    ptr = dlsym(gles_android,"eglGetProcAddress"); disp->set_eglGetProcAddress((eglGetProcAddress_t)ptr);
    ptr = dlsym(gles_android,"eglLockSurfaceKHR"); disp->set_eglLockSurfaceKHR((eglLockSurfaceKHR_t)ptr);
    ptr = dlsym(gles_android,"eglUnlockSurfaceKHR"); disp->set_eglUnlockSurfaceKHR((eglUnlockSurfaceKHR_t)ptr);
    ptr = dlsym(gles_android,"eglCreateImageKHR"); disp->set_eglCreateImageKHR((eglCreateImageKHR_t)ptr);
    ptr = dlsym(gles_android,"eglDestroyImageKHR"); disp->set_eglDestroyImageKHR((eglDestroyImageKHR_t)ptr);
    ptr = dlsym(gles_android,"eglCreateSyncKHR"); disp->set_eglCreateSyncKHR((eglCreateSyncKHR_t)ptr);
    ptr = dlsym(gles_android,"eglDestroySyncKHR"); disp->set_eglDestroySyncKHR((eglDestroySyncKHR_t)ptr);
    ptr = dlsym(gles_android,"eglClientWaitSyncKHR"); disp->set_eglClientWaitSyncKHR((eglClientWaitSyncKHR_t)ptr);
    ptr = dlsym(gles_android,"eglSignalSyncKHR"); disp->set_eglSignalSyncKHR((eglSignalSyncKHR_t)ptr);
    ptr = dlsym(gles_android,"eglGetSyncAttribKHR"); disp->set_eglGetSyncAttribKHR((eglGetSyncAttribKHR_t)ptr);
    ptr = dlsym(gles_android,"eglSetSwapRectangleANDROID"); disp->set_eglSetSwapRectangleANDROID((eglSetSwapRectangleANDROID_t)ptr);

        return disp;
}
