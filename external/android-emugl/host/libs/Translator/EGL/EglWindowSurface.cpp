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
#include "EglWindowSurface.h"
#include "EglGlobalInfo.h"
#include "EglOsApi.h"

#include "emugl/common/lazy_instance.h"

// this static member is accessed from a function registered in onexit()
// for the emulator cleanup. As all static C++ objects are also being freed
// from a system handler registered in onexit(), we can't have a class object
// by value here - it would be prone to unstable deinitialization order issue;
static emugl::LazyInstance<std::set<EGLNativeWindowType>>
    s_associatedWins = LAZY_INSTANCE_INIT;

bool EglWindowSurface::alreadyAssociatedWithConfig(EGLNativeWindowType win) {
    return s_associatedWins.get().find(win) != s_associatedWins.get().end();
}

EglWindowSurface::EglWindowSurface(EglDisplay* dpy,
                                   EGLNativeWindowType win,
                                   EglConfig* config,
                                   unsigned int width,
                                   unsigned int height) :
        EglSurface(dpy, WINDOW, config, width, height),
        m_win(win) {
    s_associatedWins.get().insert(win);
    EglOS::Engine* engine = EglGlobalInfo::getInstance()->getOsEngine();
    m_native = engine->createWindowSurface(win);
}

EglWindowSurface::~EglWindowSurface() {
    s_associatedWins.get().erase(m_win);
}

bool  EglWindowSurface::getAttrib(EGLint attrib,EGLint* val) {
    switch(attrib) {
    case EGL_CONFIG_ID:
        *val = m_config->id();
        break;
    case EGL_WIDTH:
        *val = m_width;
        break;
    case EGL_HEIGHT:
        *val = m_height;
        break;
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
