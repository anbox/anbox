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
#include "EglDisplay.h"

#include "EglConfig.h"
#include "EglOsApi.h"
#include <GLcommon/GLutils.h>

EglDisplay::EglDisplay(EGLNativeDisplayType dpy,
                       EglOS::Display* idpy) :
    m_dpy(dpy),
    m_idpy(idpy),
    m_initialized(false),
    m_configInitialized(false),
    m_nextEglImageId(0),
    m_globalSharedContext(NULL)
{
    m_manager[GLES_1_1] = new ObjectNameManager(&m_globalNameSpace);
    m_manager[GLES_2_0] = new ObjectNameManager(&m_globalNameSpace);
};

EglDisplay::~EglDisplay() {
    emugl::Mutex::AutoLock mutex(m_lock);

    //
    // Destroy the global context if one was created.
    // (should be true for windows platform only)
    //
    if (m_globalSharedContext != NULL) {
        m_idpy->destroyContext(m_globalSharedContext);
    }

    m_idpy->release();

    for(ConfigsList::iterator it = m_configs.begin();
        it != m_configs.end();
        it++) {
        delete *it;
    }

    delete m_manager[GLES_1_1];
    delete m_manager[GLES_2_0];

    delete m_idpy;
}

void EglDisplay::initialize(int renderableType) {
    emugl::Mutex::AutoLock mutex(m_lock);
    m_initialized = true;
    initConfigurations(renderableType);
    m_configInitialized = true;
}

bool EglDisplay::isInitialize() { return m_initialized;}

void EglDisplay::terminate(){
    emugl::Mutex::AutoLock mutex(m_lock);
     m_contexts.clear();
     m_surfaces.clear();
     m_initialized = false;
}

static bool compareEglConfigsPtrs(EglConfig* first,EglConfig* second) {
    return *first < *second ;
}

void EglDisplay::addSimplePixelFormat(int red_size,
                                      int green_size,
                                      int blue_size,
                                      int alpha_size) {
    m_configs.sort(compareEglConfigsPtrs);

    EGLConfig match;

    EglConfig dummy(red_size,
                    green_size,
                    blue_size,
                    alpha_size,  // RGB_565
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    16, // Depth
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    EGL_DONT_CARE,
                    NULL);

    if(!doChooseConfigs(dummy, &match, 1))
    {
        return;
    }

    const EglConfig* config = (EglConfig*)match;

    int bSize;
    config->getConfAttrib(EGL_BUFFER_SIZE,&bSize);

    if(bSize == 16)
    {
        return;
    }

    int max_config_id = 0;

    for(ConfigsList::iterator it = m_configs.begin(); it != m_configs.end() ;it++) {
        EGLint id;
        (*it)->getConfAttrib(EGL_CONFIG_ID, &id);
        if(id > max_config_id)
            max_config_id = id;
    }

    EglConfig* newConfig = new EglConfig(*config,max_config_id+1,
                                         red_size, green_size, blue_size,
                                         alpha_size);

    m_configs.push_back(newConfig);
}

void EglDisplay::addMissingConfigs() {
    addSimplePixelFormat(5, 6, 5, 0); // RGB_565
    addSimplePixelFormat(8, 8, 8, 0); // RGB_888
    // (Host GPUs that are newer may not list RGB_888
    // out of the box.)
}

void EglDisplay::initConfigurations(int renderableType) {
    if (m_configInitialized) {
        return;
    }
    m_idpy->queryConfigs(renderableType, addConfig, this);

    addMissingConfigs();
    m_configs.sort(compareEglConfigsPtrs);

#if EMUGL_DEBUG
    for (ConfigsList::const_iterator it = m_configs.begin();
         it != m_configs.end();
         it++) {
        EglConfig* config = *it;
        EGLint red, green, blue, alpha, depth, stencil, renderable, surface;
        config->getConfAttrib(EGL_RED_SIZE, &red);
        config->getConfAttrib(EGL_GREEN_SIZE, &green);
        config->getConfAttrib(EGL_BLUE_SIZE, &blue);
        config->getConfAttrib(EGL_ALPHA_SIZE, &alpha);
        config->getConfAttrib(EGL_DEPTH_SIZE, &depth);
        config->getConfAttrib(EGL_STENCIL_SIZE, &stencil);
        config->getConfAttrib(EGL_RENDERABLE_TYPE, &renderable);
        config->getConfAttrib(EGL_SURFACE_TYPE, &surface);
    }
#endif  // EMUGL_DEBUG
}

EglConfig* EglDisplay::getConfig(EGLConfig conf) const {
    emugl::Mutex::AutoLock mutex(m_lock);

    for(ConfigsList::const_iterator it = m_configs.begin();
        it != m_configs.end();
        it++) {
        if(static_cast<EGLConfig>(*it) == conf) {
            return (*it);
        }
    }
    return NULL;
}

SurfacePtr EglDisplay::getSurface(EGLSurface surface) const {
    emugl::Mutex::AutoLock mutex(m_lock);
    /* surface is "key" in map<unsigned int, SurfacePtr>. */
    unsigned int hndl = SafeUIntFromPointer(surface);
    SurfacesHndlMap::const_iterator it = m_surfaces.find(hndl);
    return it != m_surfaces.end() ?
                                  (*it).second :
                                   SurfacePtr(NULL);
}

ContextPtr EglDisplay::getContext(EGLContext ctx) const {
    emugl::Mutex::AutoLock mutex(m_lock);
    /* ctx is "key" in map<unsigned int, ContextPtr>. */
    unsigned int hndl = SafeUIntFromPointer(ctx);
    ContextsHndlMap::const_iterator it = m_contexts.find(hndl);
    return it != m_contexts.end() ?
                                  (*it).second :
                                   ContextPtr(NULL);
}

bool EglDisplay::removeSurface(EGLSurface s) {
    emugl::Mutex::AutoLock mutex(m_lock);
    /* s is "key" in map<unsigned int, SurfacePtr>. */
    unsigned int hndl = SafeUIntFromPointer(s);
    SurfacesHndlMap::iterator it = m_surfaces.find(hndl);
    if(it != m_surfaces.end()) {
        m_surfaces.erase(it);
        return true;
    }
    return false;
}

bool EglDisplay::removeContext(EGLContext ctx) {
    emugl::Mutex::AutoLock mutex(m_lock);
    /* ctx is "key" in map<unsigned int, ContextPtr>. */
    unsigned int hndl = SafeUIntFromPointer(ctx);
    ContextsHndlMap::iterator it = m_contexts.find(hndl);
    if(it != m_contexts.end()) {
        m_contexts.erase(it);
        return true;
    }
    return false;
}

bool EglDisplay::removeContext(ContextPtr ctx) {
    emugl::Mutex::AutoLock mutex(m_lock);

    ContextsHndlMap::iterator it;
    for(it = m_contexts.begin(); it != m_contexts.end();it++) {
        if((*it).second.Ptr() == ctx.Ptr()){
            break;
        }
    }
    if(it != m_contexts.end()) {
        m_contexts.erase(it);
        return true;
    }
    return false;
}

EglConfig* EglDisplay::getConfig(EGLint id) const {
    emugl::Mutex::AutoLock mutex(m_lock);

    for(ConfigsList::const_iterator it = m_configs.begin();
        it != m_configs.end();
        it++) {
        if((*it)->id() == id) {
            return (*it);

        }
    }
    return NULL;
}

int EglDisplay::getConfigs(EGLConfig* configs,int config_size) const {
    emugl::Mutex::AutoLock mutex(m_lock);
    int i = 0;
    for(ConfigsList::const_iterator it = m_configs.begin();
        it != m_configs.end() && i < config_size;
        i++, it++) {
        configs[i] = static_cast<EGLConfig>(*it);
    }
    return i;
}

int EglDisplay::chooseConfigs(const EglConfig& dummy,
                              EGLConfig* configs,
                              int config_size) const {
    emugl::Mutex::AutoLock mutex(m_lock);
    return doChooseConfigs(dummy, configs, config_size);
}

int EglDisplay::doChooseConfigs(const EglConfig& dummy,
                                EGLConfig* configs,
                                int config_size) const {
    int added = 0;
    for(ConfigsList::const_iterator it = m_configs.begin();
        it != m_configs.end() && (added < config_size || !configs);
        it++) {
        if( (*it)->chosen(dummy)){
            if(configs) {
                configs[added] = static_cast<EGLConfig>(*it);
            }
            added++;
       }
    }
    //no need to sort since the configurations are saved already in sorted maner
    return added;
}

EGLSurface EglDisplay::addSurface(SurfacePtr s ) {
   emugl::Mutex::AutoLock mutex(m_lock);
   unsigned int hndl = s.Ptr()->getHndl();
   EGLSurface ret =reinterpret_cast<EGLSurface> (hndl);

   if(m_surfaces.find(hndl) != m_surfaces.end()) {
       return ret;
   }

   m_surfaces[hndl] = s;
   return ret;
}

EGLContext EglDisplay::addContext(ContextPtr ctx ) {
    emugl::Mutex::AutoLock mutex(m_lock);

   unsigned int hndl = ctx.Ptr()->getHndl();
   EGLContext ret    = reinterpret_cast<EGLContext> (hndl);

   if(m_contexts.find(hndl) != m_contexts.end()) {
       return ret;
   }
   m_contexts[hndl] = ctx;
   return ret;
}


EGLImageKHR EglDisplay::addImageKHR(ImagePtr img) {
    emugl::Mutex::AutoLock mutex(m_lock);
    do { ++m_nextEglImageId; } while(m_nextEglImageId == 0);
    img->imageId = m_nextEglImageId;
    m_eglImages[m_nextEglImageId] = img;
    return reinterpret_cast<EGLImageKHR>(m_nextEglImageId);
}

ImagePtr EglDisplay::getImage(EGLImageKHR img) const {
    emugl::Mutex::AutoLock mutex(m_lock);
    /* img is "key" in map<unsigned int, ImagePtr>. */
    unsigned int hndl = SafeUIntFromPointer(img);
    ImagesHndlMap::const_iterator i( m_eglImages.find(hndl) );
    return (i != m_eglImages.end()) ? (*i).second :ImagePtr(NULL);
}

bool EglDisplay:: destroyImageKHR(EGLImageKHR img) {
    emugl::Mutex::AutoLock mutex(m_lock);
    /* img is "key" in map<unsigned int, ImagePtr>. */
    unsigned int hndl = SafeUIntFromPointer(img);
    ImagesHndlMap::iterator i( m_eglImages.find(hndl) );
    if (i != m_eglImages.end())
    {
        m_eglImages.erase(i);
        return true;
    }
    return false;
}

EglOS::Context* EglDisplay::getGlobalSharedContext() const {
    emugl::Mutex::AutoLock mutex(m_lock);
#ifndef _WIN32
    // find an existing OpenGL context to share with, if exist
    EglOS::Context* ret =
        (EglOS::Context*)m_manager[GLES_1_1]->getGlobalContext();
    if (!ret)
        ret = (EglOS::Context*)m_manager[GLES_2_0]->getGlobalContext();
    return ret;
#else
    if (!m_globalSharedContext) {
        //
        // On windows we create a dummy context to serve as the
        // "global context" which all contexts share with.
        // This is because on windows it is not possible to share
        // with a context which is already current. This dummy context
        // will never be current to any thread so it is safe to share with.
        // Create that context using the first config
        if (m_configs.size() < 1) {
            // Should not happen! config list should be initialized at this point
            return NULL;
        }
        EglConfig *cfg = (*m_configs.begin());
        m_globalSharedContext = m_idpy->createContext(
                cfg->nativeFormat(), NULL);
    }

    return m_globalSharedContext;
#endif
}

// static
void EglDisplay::addConfig(void* opaque, const EglOS::ConfigInfo* info) {
    EglDisplay* display = static_cast<EglDisplay*>(opaque);
    EglConfig* config = new EglConfig(
            info->red_size,
            info->green_size,
            info->blue_size,
            info->alpha_size,
            info->caveat,
            info->config_id,
            info->depth_size,
            info->frame_buffer_level,
            info->max_pbuffer_width,
            info->max_pbuffer_height,
            info->max_pbuffer_size,
            info->native_renderable,
            info->renderable_type,
            info->native_visual_id,
            info->native_visual_type,
            info->samples_per_pixel,
            info->stencil_size,
            info->surface_type,
            info->transparent_type,
            info->trans_red_val,
            info->trans_green_val,
            info->trans_blue_val,
            info->frmt);

    display->m_configs.push_back(config);
}
