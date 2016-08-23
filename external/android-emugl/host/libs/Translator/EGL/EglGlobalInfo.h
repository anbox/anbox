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
#ifndef EGL_GLOBAL_INFO
#define EGL_GLOBAL_INFO

#include "EglDisplay.h"
#include "EglConfig.h"
#include "EglContext.h"
#include "EglOsApi.h"

#include "emugl/common/lazy_instance.h"
#include "emugl/common/pod_vector.h"
#include "emugl/common/mutex.h"

#include <GLcommon/TranslatorIfaces.h>

#include <EGL/egl.h>

class EglDisplay;

// Holds all global information shared by the EGL implementation in a given
// process. This really amounts to:
//
//   - A list of EglDisplay instances, each identified by an
//     EGLNativeDisplayType and EglOS::Display*.
//
//   - GLES interface function pointers for all supported GLES versions.

class EglGlobalInfo {

public:
    // Returns a pointer to the process' single instance, which will be
    // created on demand. This can be called multiple times, each call will
    // increment an internal reference-count.
    static EglGlobalInfo* getInstance();

    // Create a new EglDisplay instance from an existing native |dpy| value.
    // |idpy| is the corresponding native internal display type. See
    // generateInternalDisplay() below to understand how they differ.
    EglDisplay* addDisplay(EGLNativeDisplayType dpy,
                           EglOS::Display* idpy);

    // Return the EglDisplay instance corresponding to a given native |dpy|
    // value.
    EglDisplay* getDisplay(EGLNativeDisplayType dpy) const;

    // Return the EglDisplay instance corresponding to a given EGLDisplay |dpy|
    // value. NULL if none matches.
    EglDisplay* getDisplay(EGLDisplay dpy) const;

    // Remove a given EGLDisplay identified by |dpy|.
    bool removeDisplay(EGLDisplay dpy);

    // Return the default native internal display handle.
    EglOS::Display* getDefaultNativeDisplay() const {
        return m_display;
    };

    // Return the default engine handle.
    EglOS::Engine* getOsEngine() const {
        return m_engine;
    }

    // Set the GLES interface pointer corresponding to a given GLES version.
    // |iface| is a pointer to a structure containing function pointers
    // related to a specific GLES version.
    // |ver| is a version identifier, e.g. GLES_1_1 or GLES_2_0.
    void setIface(const GLESiface* iface, GLESVersion ver) {
        m_gles_ifaces[ver] = iface;
    };

    // Return the current GLES interface pointer for a given GLES version.
    // |ver| is a version identifier, e.g. GLES_1_1 or GLES_2_0.
    const GLESiface* getIface(GLESVersion ver) const {
        return m_gles_ifaces[ver];
    }

    // Initialize the table of extension functions for a given GLES version
    // |ver|. This must be called after setIface() for the corresponding
    // version.
    void initClientExtFuncTable(GLESVersion ver);

private:
    EglGlobalInfo();
    ~EglGlobalInfo();

    friend struct emugl::LazyInstance<EglGlobalInfo>;

    emugl::PodVector<EglDisplay*>  m_displays;
    EglOS::Engine*                 m_engine;
    EglOS::Display*                m_display;
    const GLESiface*               m_gles_ifaces[MAX_GLES_VERSION];
    bool                           m_gles_extFuncs_inited[MAX_GLES_VERSION];
    mutable emugl::Mutex           m_lock;
};

#endif
