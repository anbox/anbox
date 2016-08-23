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
#ifndef EGL_SURFACE_H
#define EGL_SURFACE_H

#include <map>

#include "EglConfig.h"
#include "EglOsApi.h"

#include "emugl/common/smart_ptr.h"

#include <EGL/egl.h>

class EglSurface;
typedef emugl::SmartPtr<EglSurface> SurfacePtr;

class EglDisplay;

class EglSurface {
public:
    typedef enum {
        WINDOW  = 0,
        PBUFFER = 1,
        PIXMAP  = 3
    } ESurfaceType;

    ESurfaceType type() const { return m_type; }

    EglOS::Surface* native() const { return m_native; }

    virtual bool setAttrib(EGLint attrib, EGLint val);
    virtual bool getAttrib(EGLint attrib, EGLint* val) = 0;

    void setDim(int width, int height) {
        m_width = width;
        m_height = height;
    }

    EglConfig* getConfig() const { return m_config; }

    unsigned int getHndl() const { return m_hndl; }

    virtual ~EglSurface();

private:
    static unsigned int s_nextSurfaceHndl;

    ESurfaceType m_type;
    unsigned int m_hndl;

protected:
    EglSurface(EglDisplay *dpy,
               ESurfaceType type,
               EglConfig* config,
               EGLint width,
               EGLint height) :
       m_type(type),
       m_config(config),
       m_width(width),
       m_height(height),
       m_native(NULL),
       m_dpy(dpy)
    {
        m_hndl = ++s_nextSurfaceHndl;
    }

protected:
    EglConfig*            m_config;
    EGLint                m_width;
    EGLint                m_height;
    EglOS::Surface*       m_native;
    EglDisplay           *m_dpy;
};

#endif  // EGL_SURFACE_H
