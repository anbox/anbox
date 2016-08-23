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
#ifndef EGL_PBUFFER_SURFACE_H
#define EGL_PBUFFER_SURFACE_H

#include "EglSurface.h"

class EglDisplay;

class EglPbufferSurface:public EglSurface {
public:
    EglPbufferSurface(EglDisplay *dpy, EglConfig* config):
                                         EglSurface(dpy,PBUFFER,config,0,0),
                                         m_texFormat(EGL_NO_TEXTURE),
                                         m_texTarget(EGL_NO_TEXTURE),
                                         m_texMipmap(EGL_FALSE),
                                         m_largest(EGL_FALSE){};

    void  setNativePbuffer(EglOS::Surface* srfc) { m_native = srfc; }
    bool  setAttrib(EGLint attrib,EGLint val);
    bool  getAttrib(EGLint attrib,EGLint* val);
    void  getDim(EGLint* width,EGLint* height,EGLint* largest){
                                                              *width = m_width;
                                                              *height = m_height;
                                                              *largest = m_largest;
                                                             };

    void getTexInfo(EGLint* format,EGLint* target){ *format = m_texFormat; *target = m_texTarget;}

private:
    EGLint               m_texFormat;
    EGLint               m_texTarget;
    EGLint               m_texMipmap;
    EGLint               m_largest;
};
#endif
