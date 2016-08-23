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
#ifndef EGL_CONTEXT_H
#define EGL_CONTEXT_H

#include "EglConfig.h"
#include "EglOsApi.h"
#include "EglSurface.h"

#include <GLcommon/GLutils.h>
#include <GLcommon/TranslatorIfaces.h>
#include <GLcommon/objectNameManager.h>

#include "emugl/common/smart_ptr.h"

#include <EGL/egl.h>

#include <map>

class EglContext;
typedef  emugl::SmartPtr<EglContext> ContextPtr;

class EglDisplay;

class EglContext {

public:

    EglContext(EglDisplay *dpy, EglOS::Context* context,ContextPtr shared_context,EglConfig* config,GLEScontext* glesCtx,GLESVersion ver,ObjectNameManager* mngr);
    bool usingSurface(SurfacePtr surface);
    EglOS::Context* nativeType() const { return m_native; }
    bool getAttrib(EGLint attrib,EGLint* value);
    SurfacePtr read(){ return m_read;};
    SurfacePtr draw(){ return m_draw;};
    ShareGroupPtr getShareGroup(){return m_shareGroup;}
    EglConfig* getConfig(){ return m_config;};
    GLESVersion version(){return m_version;};
    GLEScontext* getGlesContext(){return m_glesContext;}
    void setSurfaces(SurfacePtr read,SurfacePtr draw);
    unsigned int getHndl(){return m_hndl;}
    bool attachImage(unsigned int imageId,ImagePtr img);
    void detachImage(unsigned int imageId);

    ~EglContext();

private:
    static unsigned int  s_nextContextHndl;
    EglDisplay          *m_dpy;
    EglOS::Context*      m_native;
    EglConfig*           m_config;
    GLEScontext*         m_glesContext;
    ShareGroupPtr        m_shareGroup;
    SurfacePtr           m_read;
    SurfacePtr           m_draw;
    GLESVersion          m_version;
    ObjectNameManager    *m_mngr;
    unsigned int         m_hndl;
    ImagesHndlMap        m_attachedImages;
};

#endif
