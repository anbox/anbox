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
#include "EglSurface.h"

#include "EglDisplay.h"
#include "EglOsApi.h"

unsigned int EglSurface::s_nextSurfaceHndl = 0;

EglSurface::~EglSurface(){ 

    if(m_type == EglSurface::PBUFFER) {
        m_dpy->nativeType()->releasePbuffer(m_native);
    }

    delete m_native;
}

bool  EglSurface::setAttrib(EGLint attrib,EGLint val) {
    switch(attrib) {
    case EGL_WIDTH:
    case EGL_HEIGHT:
    case EGL_LARGEST_PBUFFER:
    case EGL_TEXTURE_FORMAT:
    case EGL_TEXTURE_TARGET:
    case EGL_MIPMAP_TEXTURE:
        break;
    default:
        return false;
    }
    return true;
}
