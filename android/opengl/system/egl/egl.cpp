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

#include "HostConnection.h"
#include "ThreadInfo.h"
#include "eglDisplay.h"
#include "egl_ftable.h"
#include <cutils/log.h>
#include "gralloc_cb.h"
#include "GLClientState.h"
#include "GLSharedGroup.h"
#include "eglContext.h"
#include "ClientAPIExts.h"
#include "EGLImage.h"

#include "GLEncoder.h"
#ifdef WITH_GLES2
#include "GL2Encoder.h"
#endif

#include <system/window.h>

template<typename T>
static T setErrorFunc(GLint error, T returnValue) {
    getEGLThreadInfo()->eglError = error;
    return returnValue;
}

const char *  eglStrError(EGLint err)
{
    switch (err){
        case EGL_SUCCESS:           return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED:   return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS:        return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC:         return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE:     return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONFIG:        return "EGL_BAD_CONFIG";
        case EGL_BAD_CONTEXT:       return "EGL_BAD_CONTEXT";
        case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY:       return "EGL_BAD_DISPLAY";
        case EGL_BAD_MATCH:         return "EGL_BAD_MATCH";
        case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
        case EGL_BAD_PARAMETER:     return "EGL_BAD_PARAMETER";
        case EGL_BAD_SURFACE:       return "EGL_BAD_SURFACE";
        case EGL_CONTEXT_LOST:      return "EGL_CONTEXT_LOST";
        default: return "UNKNOWN";
    }
}

#define LOG_EGL_ERRORS 1

#ifdef LOG_EGL_ERRORS

#define setErrorReturn(error, retVal)     \
    {                                                \
        ALOGE("tid %d: %s(%d): error 0x%x (%s)", gettid(), __FUNCTION__, __LINE__, error, eglStrError(error));     \
        return setErrorFunc(error, retVal);            \
    }

#define RETURN_ERROR(ret,err)           \
    ALOGE("tid %d: %s(%d): error 0x%x (%s)", gettid(), __FUNCTION__, __LINE__, err, eglStrError(err));    \
    getEGLThreadInfo()->eglError = err;    \
    return ret;

#else //!LOG_EGL_ERRORS

#define setErrorReturn(error, retVal) return setErrorFunc(error, retVal);

#define RETURN_ERROR(ret,err)           \
    getEGLThreadInfo()->eglError = err; \
    return ret;

#endif //LOG_EGL_ERRORS

#define VALIDATE_CONFIG(cfg,ret) \
    if(((intptr_t)cfg<0)||((intptr_t)cfg>s_display.getNumConfigs())) { \
        RETURN_ERROR(ret,EGL_BAD_CONFIG); \
    }

#define VALIDATE_DISPLAY(dpy,ret) \
    if ((dpy) != (EGLDisplay)&s_display) { \
        RETURN_ERROR(ret, EGL_BAD_DISPLAY);    \
    }

#define VALIDATE_DISPLAY_INIT(dpy,ret) \
    VALIDATE_DISPLAY(dpy, ret)    \
    if (!s_display.initialized()) {        \
        RETURN_ERROR(ret, EGL_NOT_INITIALIZED);    \
    }

#define DEFINE_HOST_CONNECTION \
    HostConnection *hostCon = HostConnection::get(); \
    renderControl_encoder_context_t *rcEnc = (hostCon ? hostCon->rcEncoder() : NULL)

#define DEFINE_AND_VALIDATE_HOST_CONNECTION(ret) \
    HostConnection *hostCon = HostConnection::get(); \
    if (!hostCon) { \
        ALOGE("egl: Failed to get host connection\n"); \
        return ret; \
    } \
    renderControl_encoder_context_t *rcEnc = hostCon->rcEncoder(); \
    if (!rcEnc) { \
        ALOGE("egl: Failed to get renderControl encoder context\n"); \
        return ret; \
    }

#define VALIDATE_CONTEXT_RETURN(context,ret)        \
    if (!context) {                                    \
        RETURN_ERROR(ret,EGL_BAD_CONTEXT);    \
    }

#define VALIDATE_SURFACE_RETURN(surface, ret)    \
    if (surface != EGL_NO_SURFACE) {    \
        egl_surface_t* s( static_cast<egl_surface_t*>(surface) );    \
        if (s->dpy != (EGLDisplay)&s_display)    \
            setErrorReturn(EGL_BAD_DISPLAY, EGL_FALSE);    \
    }


EGLContext_t::EGLContext_t(EGLDisplay dpy, EGLConfig config, EGLContext_t* shareCtx) :
    dpy(dpy),
    config(config),
    read(EGL_NO_SURFACE),
    draw(EGL_NO_SURFACE),
    shareCtx(shareCtx),
    rcContext(0),
    versionString(NULL),
    vendorString(NULL),
    rendererString(NULL),
    shaderVersionString(NULL),
    extensionString(NULL),
    deletePending(0)
{
    flags = 0;
    version = 1;
    clientState = new GLClientState();
    if (shareCtx)
        sharedGroup = shareCtx->getSharedGroup();
    else
        sharedGroup = GLSharedGroupPtr(new GLSharedGroup());
};

EGLContext_t::~EGLContext_t()
{
    delete clientState;
    delete [] versionString;
    delete [] vendorString;
    delete [] rendererString;
    delete [] shaderVersionString;
    delete [] extensionString;
}

// ----------------------------------------------------------------------------
//egl_surface_t

//we don't need to handle depth since it's handled when window created on the host

struct egl_surface_t {

    EGLDisplay          dpy;
    EGLConfig           config;


    egl_surface_t(EGLDisplay dpy, EGLConfig config, EGLint surfaceType);
    virtual     ~egl_surface_t();

    virtual     void        setSwapInterval(int interval) = 0;
    virtual     EGLBoolean  swapBuffers() = 0;

    EGLint      getSwapBehavior() const;
    uint32_t    getRcSurface()   { return rcSurface; }
    EGLint      getSurfaceType() { return surfaceType; }

    EGLint      getWidth(){ return width; }
    EGLint      getHeight(){ return height; }
    void        setTextureFormat(EGLint _texFormat) { texFormat = _texFormat; }
    EGLint      getTextureFormat() { return texFormat; }
    void        setTextureTarget(EGLint _texTarget) { texTarget = _texTarget; }
    EGLint      getTextureTarget() { return texTarget; }

private:
    //
    //Surface attributes
    //
    EGLint      width;
    EGLint      height;
    EGLint      texFormat;
    EGLint      texTarget;

protected:
    void        setWidth(EGLint w)  { width = w;  }
    void        setHeight(EGLint h) { height = h; }

    EGLint      surfaceType;
    uint32_t    rcSurface; //handle to surface created via remote control
};

egl_surface_t::egl_surface_t(EGLDisplay dpy, EGLConfig config, EGLint surfaceType)
    : dpy(dpy), config(config), surfaceType(surfaceType), rcSurface(0)
{
    width = 0;
    height = 0;
    texFormat = EGL_NO_TEXTURE;
    texTarget = EGL_NO_TEXTURE;
}

EGLint egl_surface_t::getSwapBehavior() const {
    return EGL_BUFFER_PRESERVED;
}

egl_surface_t::~egl_surface_t()
{
}

// ----------------------------------------------------------------------------
// egl_window_surface_t

struct egl_window_surface_t : public egl_surface_t {
    static egl_window_surface_t* create(
            EGLDisplay dpy, EGLConfig config, EGLint surfType,
            ANativeWindow* window);

    virtual ~egl_window_surface_t();

    virtual void       setSwapInterval(int interval);
    virtual EGLBoolean swapBuffers();

private:
    egl_window_surface_t(
            EGLDisplay dpy, EGLConfig config, EGLint surfType,
            ANativeWindow* window);
    EGLBoolean init();

    ANativeWindow*              nativeWindow;
    android_native_buffer_t*    buffer;
};

egl_window_surface_t::egl_window_surface_t (
        EGLDisplay dpy, EGLConfig config, EGLint surfType,
        ANativeWindow* window)
:   egl_surface_t(dpy, config, surfType),
    nativeWindow(window),
    buffer(NULL)
{
    // keep a reference on the window
    nativeWindow->common.incRef(&nativeWindow->common);
}

EGLBoolean egl_window_surface_t::init()
{
    if (nativeWindow->dequeueBuffer_DEPRECATED(nativeWindow, &buffer) != NO_ERROR) {
        setErrorReturn(EGL_BAD_ALLOC, EGL_FALSE);
    }
    setWidth(buffer->width);
    setHeight(buffer->height);

    DEFINE_AND_VALIDATE_HOST_CONNECTION(EGL_FALSE);
    rcSurface = rcEnc->rcCreateWindowSurface(rcEnc, (uintptr_t)config,
            getWidth(), getHeight());
    if (!rcSurface) {
        ALOGE("rcCreateWindowSurface returned 0");
        return EGL_FALSE;
    }
    rcEnc->rcSetWindowColorBuffer(rcEnc, rcSurface,
            ((cb_handle_t*)(buffer->handle))->hostHandle);

    return EGL_TRUE;
}

egl_window_surface_t* egl_window_surface_t::create(
        EGLDisplay dpy, EGLConfig config, EGLint surfType,
        ANativeWindow* window)
{
    egl_window_surface_t* wnd = new egl_window_surface_t(
            dpy, config, surfType, window);
    if (wnd && !wnd->init()) {
        delete wnd;
        wnd = NULL;
    }
    return wnd;
}

egl_window_surface_t::~egl_window_surface_t() {
    DEFINE_HOST_CONNECTION;
    if (rcSurface && rcEnc) {
        rcEnc->rcDestroyWindowSurface(rcEnc, rcSurface);
    }
    if (buffer) {
        nativeWindow->cancelBuffer_DEPRECATED(nativeWindow, buffer);
    }
    nativeWindow->common.decRef(&nativeWindow->common);
}

void egl_window_surface_t::setSwapInterval(int interval)
{
    nativeWindow->setSwapInterval(nativeWindow, interval);
}

EGLBoolean egl_window_surface_t::swapBuffers()
{
    DEFINE_AND_VALIDATE_HOST_CONNECTION(EGL_FALSE);

    rcEnc->rcFlushWindowColorBuffer(rcEnc, rcSurface);

    nativeWindow->queueBuffer_DEPRECATED(nativeWindow, buffer);
    if (nativeWindow->dequeueBuffer_DEPRECATED(nativeWindow, &buffer)) {
        buffer = NULL;
        setErrorReturn(EGL_BAD_ALLOC, EGL_FALSE);
    }

    rcEnc->rcSetWindowColorBuffer(rcEnc, rcSurface,
            ((cb_handle_t *)(buffer->handle))->hostHandle);

    setWidth(buffer->width);
    setHeight(buffer->height);

    return EGL_TRUE;
}

// ----------------------------------------------------------------------------
//egl_pbuffer_surface_t

struct egl_pbuffer_surface_t : public egl_surface_t {
    static egl_pbuffer_surface_t* create(EGLDisplay dpy, EGLConfig config,
            EGLint surfType, int32_t w, int32_t h, GLenum pixelFormat);

    virtual ~egl_pbuffer_surface_t();

    virtual void       setSwapInterval(int interval) { (void)interval; }
    virtual EGLBoolean swapBuffers() { return EGL_TRUE; }

    uint32_t getRcColorBuffer() { return rcColorBuffer; }

private:
    egl_pbuffer_surface_t(EGLDisplay dpy, EGLConfig config, EGLint surfType,
            int32_t w, int32_t h);
    EGLBoolean init(GLenum format);

    uint32_t rcColorBuffer;
};

egl_pbuffer_surface_t::egl_pbuffer_surface_t(EGLDisplay dpy, EGLConfig config,
        EGLint surfType, int32_t w, int32_t h)
:   egl_surface_t(dpy, config, surfType),
    rcColorBuffer(0)
{
    setWidth(w);
    setHeight(h);
}

egl_pbuffer_surface_t::~egl_pbuffer_surface_t()
{
    DEFINE_HOST_CONNECTION;
    if (rcEnc) {
        if (rcColorBuffer) rcEnc->rcCloseColorBuffer(rcEnc, rcColorBuffer);
        if (rcSurface)     rcEnc->rcDestroyWindowSurface(rcEnc, rcSurface);
    }
}

EGLBoolean egl_pbuffer_surface_t::init(GLenum pixelFormat)
{
    DEFINE_AND_VALIDATE_HOST_CONNECTION(EGL_FALSE);

    rcSurface = rcEnc->rcCreateWindowSurface(rcEnc, (uintptr_t)config,
            getWidth(), getHeight());
    if (!rcSurface) {
        ALOGE("rcCreateWindowSurface returned 0");
        return EGL_FALSE;
    }

    rcColorBuffer = rcEnc->rcCreateColorBuffer(rcEnc, getWidth(), getHeight(),
            pixelFormat);
    if (!rcColorBuffer) {
        ALOGE("rcCreateColorBuffer returned 0");
        return EGL_FALSE;
    }

    rcEnc->rcSetWindowColorBuffer(rcEnc, rcSurface, rcColorBuffer);

    return EGL_TRUE;
}

egl_pbuffer_surface_t* egl_pbuffer_surface_t::create(EGLDisplay dpy,
        EGLConfig config, EGLint surfType, int32_t w, int32_t h,
        GLenum pixelFormat)
{
    egl_pbuffer_surface_t* pb = new egl_pbuffer_surface_t(dpy, config, surfType,
            w, h);
    if (pb && !pb->init(pixelFormat)) {
        delete pb;
        pb = NULL;
    }
    return pb;
}

static const char *getGLString(int glEnum)
{
    EGLThreadInfo *tInfo = getEGLThreadInfo();
    if (!tInfo || !tInfo->currentContext) {
        return NULL;
    }

    const char** strPtr = NULL;

#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_EXTENSIONS                     0x1F03

    switch(glEnum) {
        case GL_VERSION:
            strPtr = &tInfo->currentContext->versionString;
            break;
        case GL_VENDOR:
            strPtr = &tInfo->currentContext->vendorString;
            break;
        case GL_RENDERER:
            strPtr = &tInfo->currentContext->rendererString;
            break;
        case GL_SHADING_LANGUAGE_VERSION:
            strPtr = &tInfo->currentContext->shaderVersionString;
            break;
        case GL_EXTENSIONS:
            strPtr = &tInfo->currentContext->extensionString;
            break;
    }

    if (!strPtr) {
        return NULL;
    }

    if (*strPtr != NULL) {
        //
        // string is already cached
        //
        return *strPtr;
    }

    //
    // first query of that string - need to query host
    //
    DEFINE_AND_VALIDATE_HOST_CONNECTION(NULL);
    char *hostStr = NULL;
    int n = rcEnc->rcGetGLString(rcEnc, glEnum, NULL, 0);
    if (n < 0) {
        hostStr = new char[-n+1];
        n = rcEnc->rcGetGLString(rcEnc, glEnum, hostStr, -n);
        if (n <= 0) {
            delete [] hostStr;
            hostStr = NULL;
        }
    }

    //
    // keep the string in the context and return its value
    //
    *strPtr = hostStr;
    return hostStr;
}

// ----------------------------------------------------------------------------

// The one and only supported display object.
static eglDisplay s_display;

static EGLClient_eglInterface s_eglIface = {
    getThreadInfo: getEGLThreadInfo,
    getGLString: getGLString
};

#define DBG_FUNC DBG("%s\n", __FUNCTION__)
EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id)
{
    //
    // we support only EGL_DEFAULT_DISPLAY.
    //
    if (display_id != EGL_DEFAULT_DISPLAY) {
        return EGL_NO_DISPLAY;
    }

    return (EGLDisplay)&s_display;
}

EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
    VALIDATE_DISPLAY(dpy,EGL_FALSE);

    if (!s_display.initialize(&s_eglIface)) {
        return EGL_FALSE;
    }
    if (major!=NULL)
        *major = s_display.getVersionMajor();
    if (minor!=NULL)
        *minor = s_display.getVersionMinor();
    return EGL_TRUE;
}

EGLBoolean eglTerminate(EGLDisplay dpy)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);

    s_display.terminate();
    return EGL_TRUE;
}

EGLint eglGetError()
{
    EGLint error = getEGLThreadInfo()->eglError;
    getEGLThreadInfo()->eglError = EGL_SUCCESS;
    return error;
}

__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char *procname)
{
    // search in EGL function table
    for (int i=0; i<egl_num_funcs; i++) {
        if (!strcmp(egl_funcs_by_name[i].name, procname)) {
            return (__eglMustCastToProperFunctionPointerType)egl_funcs_by_name[i].proc;
        }
    }

    // look in gles client api's extensions table
    return (__eglMustCastToProperFunctionPointerType)ClientAPIExts::getProcAddress(procname);

    // Fail - function not found.
    return NULL;
}

const char* eglQueryString(EGLDisplay dpy, EGLint name)
{
    VALIDATE_DISPLAY_INIT(dpy, NULL);

    return s_display.queryString(name);
}

EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);

    if(!num_config) {
        RETURN_ERROR(EGL_FALSE,EGL_BAD_PARAMETER);
    }

    GLint numConfigs = s_display.getNumConfigs();
    if (!configs) {
        *num_config = numConfigs;
        return EGL_TRUE;
    }

    EGLint i;
    for (i = 0 ; i < numConfigs && i < config_size ; i++) {
        *configs++ = (EGLConfig)(uintptr_t)i;
    }
    *num_config = i;
    return EGL_TRUE;
}

EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);

    int attribs_size = 0;
    if (attrib_list) {
        const EGLint * attrib_p = attrib_list;
        while (attrib_p[0] != EGL_NONE) {
            attribs_size += 2;
            attrib_p += 2;
        }
        attribs_size++; //for the terminating EGL_NONE
    }

    uint32_t* tempConfigs[config_size];
    DEFINE_AND_VALIDATE_HOST_CONNECTION(EGL_FALSE);
    *num_config = rcEnc->rcChooseConfig(rcEnc, (EGLint*)attrib_list, attribs_size * sizeof(EGLint), (uint32_t*)tempConfigs, config_size);
    if (configs!=NULL) {
        EGLint i=0;
        for (i=0;i<(*num_config);i++) {
             *((uintptr_t*)configs+i) = *((uint32_t*)tempConfigs+i);
        }
    }

    if (*num_config <= 0)
        return EGL_FALSE;
    return EGL_TRUE;
}

EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);
    VALIDATE_CONFIG(config, EGL_FALSE);

    if (s_display.getConfigAttrib(config, attribute, value))
    {
        return EGL_TRUE;
    }
    else
    {
        RETURN_ERROR(EGL_FALSE, EGL_BAD_ATTRIBUTE);
    }
}

EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
{
    (void)attrib_list;

    VALIDATE_DISPLAY_INIT(dpy, NULL);
    VALIDATE_CONFIG(config, EGL_FALSE);
    if (win == 0) {
        setErrorReturn(EGL_BAD_MATCH, EGL_NO_SURFACE);
    }

    EGLint surfaceType;
    if (s_display.getConfigAttrib(config, EGL_SURFACE_TYPE, &surfaceType) == EGL_FALSE)    return EGL_FALSE;

    if (!(surfaceType & EGL_WINDOW_BIT)) {
        setErrorReturn(EGL_BAD_MATCH, EGL_NO_SURFACE);
    }

    if (static_cast<ANativeWindow*>(win)->common.magic != ANDROID_NATIVE_WINDOW_MAGIC) {
        setErrorReturn(EGL_BAD_NATIVE_WINDOW, EGL_NO_SURFACE);
    }

    egl_surface_t* surface = egl_window_surface_t::create(
            &s_display, config, EGL_WINDOW_BIT, static_cast<ANativeWindow*>(win));
    if (!surface) {
        setErrorReturn(EGL_BAD_ALLOC, EGL_NO_SURFACE);
    }

    return surface;
}

EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list)
{
    VALIDATE_DISPLAY_INIT(dpy, NULL);
    VALIDATE_CONFIG(config, EGL_FALSE);

    EGLint surfaceType;
    if (s_display.getConfigAttrib(config, EGL_SURFACE_TYPE, &surfaceType) == EGL_FALSE)    return EGL_FALSE;

    if (!(surfaceType & EGL_PBUFFER_BIT)) {
        setErrorReturn(EGL_BAD_MATCH, EGL_NO_SURFACE);
    }

    int32_t w = 0;
    int32_t h = 0;
    EGLint texFormat = EGL_NO_TEXTURE;
    EGLint texTarget = EGL_NO_TEXTURE;
    while (attrib_list[0] != EGL_NONE) {
        switch (attrib_list[0]) {
            case EGL_WIDTH:
                w = attrib_list[1];
                break;
            case EGL_HEIGHT:
                h = attrib_list[1];
                break;
            case EGL_TEXTURE_FORMAT:
                texFormat = attrib_list[1];
                break;
            case EGL_TEXTURE_TARGET:
                texTarget = attrib_list[1];
                break;
            default:
                break;
        };
        attrib_list+=2;
    }
    if (((texFormat == EGL_NO_TEXTURE)&&(texTarget != EGL_NO_TEXTURE)) ||
        ((texFormat != EGL_NO_TEXTURE)&&(texTarget == EGL_NO_TEXTURE))) {
        setErrorReturn(EGL_BAD_MATCH, EGL_NO_SURFACE);
    }
    // TODO: check EGL_TEXTURE_FORMAT - need to support eglBindTexImage

    GLenum pixelFormat;
    if (s_display.getConfigGLPixelFormat(config, &pixelFormat) == EGL_FALSE)
        setErrorReturn(EGL_BAD_MATCH, EGL_NO_SURFACE);

    egl_surface_t* surface = egl_pbuffer_surface_t::create(dpy, config,
            EGL_PBUFFER_BIT, w, h, pixelFormat);
    if (!surface) {
        setErrorReturn(EGL_BAD_ALLOC, EGL_NO_SURFACE);
    }

    //setup attributes
    surface->setTextureFormat(texFormat);
    surface->setTextureTarget(texTarget);

    return surface;
}

EGLSurface eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list)
{
    //XXX: Pixmap not supported. The host cannot render to a pixmap resource
    //     located on host. In order to support Pixmaps we should either punt
    //     to s/w rendering -or- let the host render to a buffer that will be
    //     copied back to guest at some sync point. None of those methods not
    //     implemented and pixmaps are not used with OpenGL anyway ...
    (void)dpy;
    (void)config;
    (void)pixmap;
    (void)attrib_list;
    return EGL_NO_SURFACE;
}

EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface eglSurface)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);
    VALIDATE_SURFACE_RETURN(eglSurface, EGL_FALSE);

    egl_surface_t* surface(static_cast<egl_surface_t*>(eglSurface));
    delete surface;

    return EGL_TRUE;
}

EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface eglSurface, EGLint attribute, EGLint *value)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);
    VALIDATE_SURFACE_RETURN(eglSurface, EGL_FALSE);

    egl_surface_t* surface( static_cast<egl_surface_t*>(eglSurface) );
    EGLBoolean ret = EGL_TRUE;
    switch (attribute) {
        case EGL_CONFIG_ID:
            ret = s_display.getConfigAttrib(surface->config, EGL_CONFIG_ID, value);
            break;
        case EGL_WIDTH:
            *value = surface->getWidth();
            break;
        case EGL_HEIGHT:
            *value = surface->getHeight();
            break;
        case EGL_TEXTURE_FORMAT:
            *value = surface->getTextureFormat();
            break;
        case EGL_TEXTURE_TARGET:
            *value = surface->getTextureTarget();
            break;
        case EGL_SWAP_BEHAVIOR:
            *value = surface->getSwapBehavior();
            break;
        case EGL_LARGEST_PBUFFER:
            // not modified for a window or pixmap surface
            // and we ignore it when creating a PBuffer surface (default is EGL_FALSE)
            if (surface->getSurfaceType() & EGL_PBUFFER_BIT) *value = EGL_FALSE;
            break;
        case EGL_MIPMAP_LEVEL:
            // not modified for a window or pixmap surface
            // and we ignore it when creating a PBuffer surface (default is 0)
            if (surface->getSurfaceType() & EGL_PBUFFER_BIT) *value = 0;
            break;
        case EGL_MULTISAMPLE_RESOLVE:
            // ignored when creating the surface, return default
            *value = EGL_MULTISAMPLE_RESOLVE_DEFAULT;
            break;
        //TODO: complete other attributes
        default:
            ALOGE("eglQuerySurface %x  EGL_BAD_ATTRIBUTE", attribute);
            ret = setErrorFunc(EGL_BAD_ATTRIBUTE, EGL_FALSE);
            break;
    }

    return ret;
}

EGLBoolean eglBindAPI(EGLenum api)
{
    if (api != EGL_OPENGL_ES_API)
        setErrorReturn(EGL_BAD_PARAMETER, EGL_FALSE);
    return EGL_TRUE;
}

EGLenum eglQueryAPI()
{
    return EGL_OPENGL_ES_API;
}

EGLBoolean eglWaitClient()
{
    return eglWaitGL();
}

EGLBoolean eglReleaseThread()
{
    EGLThreadInfo *tInfo = getEGLThreadInfo();
    if (tInfo && tInfo->currentContext) {
        return eglMakeCurrent(&s_display, EGL_NO_CONTEXT, EGL_NO_SURFACE, EGL_NO_SURFACE);
    }
    return EGL_TRUE;
}

EGLSurface eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list)
{
    //TODO
    (void)dpy;
    (void)buftype;
    (void)buffer;
    (void)config;
    (void)attrib_list;
    ALOGW("%s not implemented", __FUNCTION__);
    return 0;
}

EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)
{
    // Right now we don't do anything when using host GPU.
    // This is purely just to pass the data through
    // without issuing a warning. We may benefit from validating the
    // display and surface for debug purposes.
    // TODO: Find cases where we actually need to do something.
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);
    VALIDATE_SURFACE_RETURN(surface, EGL_FALSE);
    if (surface == EGL_NO_SURFACE) {
        setErrorReturn(EGL_BAD_SURFACE, EGL_FALSE);
    }

    (void)value;

    switch (attribute) {
    case EGL_MIPMAP_LEVEL:
    case EGL_MULTISAMPLE_RESOLVE:
    case EGL_SWAP_BEHAVIOR:
        return true;
        break;
    default:
        ALOGW("%s: attr=0x%x not implemented", __FUNCTION__, attribute);
    }
    return false;
}

EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface eglSurface, EGLint buffer)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);
    VALIDATE_SURFACE_RETURN(eglSurface, EGL_FALSE);
    if (eglSurface == EGL_NO_SURFACE) {
        setErrorReturn(EGL_BAD_SURFACE, EGL_FALSE);
    }

    if (buffer != EGL_BACK_BUFFER) {
        setErrorReturn(EGL_BAD_PARAMETER, EGL_FALSE);
    }

    egl_surface_t* surface( static_cast<egl_surface_t*>(eglSurface) );

    if (surface->getTextureFormat() == EGL_NO_TEXTURE) {
        setErrorReturn(EGL_BAD_MATCH, EGL_FALSE);
    }

    if (!(surface->getSurfaceType() & EGL_PBUFFER_BIT)) {
        setErrorReturn(EGL_BAD_SURFACE, EGL_FALSE);
    }

    //It's now safe to cast to pbuffer surface
    egl_pbuffer_surface_t* pbSurface = (egl_pbuffer_surface_t*)surface;

    DEFINE_AND_VALIDATE_HOST_CONNECTION(EGL_FALSE);
    rcEnc->rcBindTexture(rcEnc, pbSurface->getRcColorBuffer());

    return GL_TRUE;
}

EGLBoolean eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    //TODO
    (void)dpy;
    (void)surface;
    (void)buffer;
    ALOGW("%s not implemented", __FUNCTION__);
    return 0;
}

EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);
    DEFINE_AND_VALIDATE_HOST_CONNECTION(EGL_FALSE);

    EGLContext_t* ctx = getEGLThreadInfo()->currentContext;
    if (!ctx) {
        setErrorReturn(EGL_BAD_CONTEXT, EGL_FALSE);
    }
    if (!ctx->draw) {
        setErrorReturn(EGL_BAD_SURFACE, EGL_FALSE);
    }
    egl_surface_t* draw(static_cast<egl_surface_t*>(ctx->draw));
    draw->setSwapInterval(interval);

    rcEnc->rcFBSetSwapInterval(rcEnc, interval); //TODO: implement on the host

    return EGL_TRUE;
}

EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_NO_CONTEXT);
    VALIDATE_CONFIG(config, EGL_NO_CONTEXT);

    EGLint version = 1; //default
    while (attrib_list && attrib_list[0] != EGL_NONE) {
        if (attrib_list[0] == EGL_CONTEXT_CLIENT_VERSION) version = attrib_list[1];
        attrib_list+=2;
    }

    // Currently only support GLES1 and 2
    if (version != 1 && version != 2) {
        setErrorReturn(EGL_BAD_CONFIG, EGL_NO_CONTEXT);
    }

    uint32_t rcShareCtx = 0;
    EGLContext_t * shareCtx = NULL;
    if (share_context) {
        shareCtx = static_cast<EGLContext_t*>(share_context);
        rcShareCtx = shareCtx->rcContext;
        if (shareCtx->dpy != dpy)
            setErrorReturn(EGL_BAD_MATCH, EGL_NO_CONTEXT);
    }

    DEFINE_AND_VALIDATE_HOST_CONNECTION(EGL_NO_CONTEXT);
    uint32_t rcContext = rcEnc->rcCreateContext(rcEnc, (uintptr_t)config, rcShareCtx, version);
    if (!rcContext) {
        ALOGE("rcCreateContext returned 0");
        setErrorReturn(EGL_BAD_ALLOC, EGL_NO_CONTEXT);
    }

    EGLContext_t * context = new EGLContext_t(dpy, config, shareCtx);
    if (!context)
        setErrorReturn(EGL_BAD_ALLOC, EGL_NO_CONTEXT);

    context->version = version;
    context->rcContext = rcContext;


    return context;
}

EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);
    VALIDATE_CONTEXT_RETURN(ctx, EGL_FALSE);

    EGLContext_t * context = static_cast<EGLContext_t*>(ctx);

    if (!context) return EGL_TRUE;

    if (getEGLThreadInfo()->currentContext == context) {
        getEGLThreadInfo()->currentContext->deletePending = 1;
        return EGL_TRUE;
    }

    if (context->rcContext) {
        DEFINE_AND_VALIDATE_HOST_CONNECTION(EGL_FALSE);
        rcEnc->rcDestroyContext(rcEnc, context->rcContext);
        context->rcContext = 0;
    }

    delete context;
    return EGL_TRUE;
}

EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);
    VALIDATE_SURFACE_RETURN(draw, EGL_FALSE);
    VALIDATE_SURFACE_RETURN(read, EGL_FALSE);

    if ((read == EGL_NO_SURFACE && draw == EGL_NO_SURFACE) && (ctx != EGL_NO_CONTEXT))
        setErrorReturn(EGL_BAD_MATCH, EGL_FALSE);
    if ((read != EGL_NO_SURFACE || draw != EGL_NO_SURFACE) && (ctx == EGL_NO_CONTEXT))
        setErrorReturn(EGL_BAD_MATCH, EGL_FALSE);

    EGLContext_t * context = static_cast<EGLContext_t*>(ctx);
    uint32_t ctxHandle = (context) ? context->rcContext : 0;
    egl_surface_t * drawSurf = static_cast<egl_surface_t *>(draw);
    uint32_t drawHandle = (drawSurf) ? drawSurf->getRcSurface() : 0;
    egl_surface_t * readSurf = static_cast<egl_surface_t *>(read);
    uint32_t readHandle = (readSurf) ? readSurf->getRcSurface() : 0;

    //
    // Nothing to do if no binding change has made
    //
    EGLThreadInfo *tInfo = getEGLThreadInfo();

    if (tInfo->currentContext == context &&
        (context == NULL ||
        (context && context->draw == draw && context->read == read))) {
        return EGL_TRUE;
    }

    if (tInfo->currentContext && tInfo->currentContext->deletePending) {
        if (tInfo->currentContext != context) {
            EGLContext_t * contextToDelete = tInfo->currentContext;
            tInfo->currentContext = 0;
            eglDestroyContext(dpy, contextToDelete);
        }
    }

    if (context && (context->flags & EGLContext_t::IS_CURRENT) && (context != tInfo->currentContext)) {
        //context is current to another thread
        setErrorReturn(EGL_BAD_ACCESS, EGL_FALSE);
    }

    DEFINE_AND_VALIDATE_HOST_CONNECTION(EGL_FALSE);
    if (rcEnc->rcMakeCurrent(rcEnc, ctxHandle, drawHandle, readHandle) == EGL_FALSE) {
        ALOGE("rcMakeCurrent returned EGL_FALSE");
        setErrorReturn(EGL_BAD_CONTEXT, EGL_FALSE);
    }

    //Now make the local bind
    if (context) {
        context->draw = draw;
        context->read = read;
        context->flags |= EGLContext_t::IS_CURRENT;
        //set the client state
        if (context->version == 2) {
            hostCon->gl2Encoder()->setClientState(context->getClientState());
            hostCon->gl2Encoder()->setSharedGroup(context->getSharedGroup());
        }
        else {
            hostCon->glEncoder()->setClientState(context->getClientState());
            hostCon->glEncoder()->setSharedGroup(context->getSharedGroup());
        }
    }
    else if (tInfo->currentContext) {
        //release ClientState & SharedGroup
        if (tInfo->currentContext->version == 2) {
            hostCon->gl2Encoder()->setClientState(NULL);
            hostCon->gl2Encoder()->setSharedGroup(GLSharedGroupPtr(NULL));
        }
        else {
            hostCon->glEncoder()->setClientState(NULL);
            hostCon->glEncoder()->setSharedGroup(GLSharedGroupPtr(NULL));
        }

    }

    if (tInfo->currentContext)
        tInfo->currentContext->flags &= ~EGLContext_t::IS_CURRENT;

    //Now make current
    tInfo->currentContext = context;

    //Check maybe we need to init the encoder, if it's first eglMakeCurrent
    if (tInfo->currentContext) {
        if (tInfo->currentContext->version == 2) {
            if (!hostCon->gl2Encoder()->isInitialized()) {
                s_display.gles2_iface()->init();
                hostCon->gl2Encoder()->setInitialized();
                ClientAPIExts::initClientFuncs(s_display.gles2_iface(), 1);
            }
        }
        else {
            if (!hostCon->glEncoder()->isInitialized()) {
                s_display.gles_iface()->init();
                hostCon->glEncoder()->setInitialized();
                ClientAPIExts::initClientFuncs(s_display.gles_iface(), 0);
            }
        }
    }

    return EGL_TRUE;
}

EGLContext eglGetCurrentContext()
{
    return getEGLThreadInfo()->currentContext;
}

EGLSurface eglGetCurrentSurface(EGLint readdraw)
{
    EGLContext_t * context = getEGLThreadInfo()->currentContext;
    if (!context)
        return EGL_NO_SURFACE; //not an error

    switch (readdraw) {
        case EGL_READ:
            return context->read;
        case EGL_DRAW:
            return context->draw;
        default:
            setErrorReturn(EGL_BAD_PARAMETER, EGL_NO_SURFACE);
    }
}

EGLDisplay eglGetCurrentDisplay()
{
    EGLContext_t * context = getEGLThreadInfo()->currentContext;
    if (!context)
        return EGL_NO_DISPLAY; //not an error

    return context->dpy;
}

EGLBoolean eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);
    VALIDATE_CONTEXT_RETURN(ctx, EGL_FALSE);

    EGLContext_t * context = static_cast<EGLContext_t*>(ctx);

    EGLBoolean ret = EGL_TRUE;
    switch (attribute) {
        case EGL_CONFIG_ID:
            ret = s_display.getConfigAttrib(context->config, EGL_CONFIG_ID, value);
            break;
        case EGL_CONTEXT_CLIENT_TYPE:
            *value = EGL_OPENGL_ES_API;
            break;
        case EGL_CONTEXT_CLIENT_VERSION:
            *value = context->version;
            break;
        case EGL_RENDER_BUFFER:
            if (!context->draw)
                *value = EGL_NONE;
            else
                *value = EGL_BACK_BUFFER; //single buffer not supported
            break;
        default:
            ALOGE("eglQueryContext %x  EGL_BAD_ATTRIBUTE", attribute);
            setErrorReturn(EGL_BAD_ATTRIBUTE, EGL_FALSE);
            break;
    }

    return ret;
}

EGLBoolean eglWaitGL()
{
    EGLThreadInfo *tInfo = getEGLThreadInfo();
    if (!tInfo || !tInfo->currentContext) {
        return EGL_FALSE;
    }

    if (tInfo->currentContext->version == 2) {
        s_display.gles2_iface()->finish();
    }
    else {
        s_display.gles_iface()->finish();
    }

    return EGL_TRUE;
}

EGLBoolean eglWaitNative(EGLint engine)
{
    (void)engine;
    return EGL_TRUE;
}

EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface eglSurface)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);
    if (eglSurface == EGL_NO_SURFACE)
        setErrorReturn(EGL_BAD_SURFACE, EGL_FALSE);

    DEFINE_AND_VALIDATE_HOST_CONNECTION(EGL_FALSE);

    egl_surface_t* d = static_cast<egl_surface_t*>(eglSurface);
    if (d->dpy != dpy)
        setErrorReturn(EGL_BAD_DISPLAY, EGL_FALSE);

    // post the surface
    d->swapBuffers();

    hostCon->flush();
    return EGL_TRUE;
}

EGLBoolean eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)
{
    //TODO :later
    (void)dpy;
    (void)surface;
    (void)target;
    return 0;
}

EGLBoolean eglLockSurfaceKHR(EGLDisplay display, EGLSurface surface, const EGLint *attrib_list)
{
    //TODO later
    (void)display;
    (void)surface;
    (void)attrib_list;
    return 0;
}

EGLBoolean eglUnlockSurfaceKHR(EGLDisplay display, EGLSurface surface)
{
    //TODO later
    (void)display;
    (void)surface;
    return 0;
}

EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
    (void)attrib_list;

    VALIDATE_DISPLAY_INIT(dpy, EGL_NO_IMAGE_KHR);

    if (target == EGL_NATIVE_BUFFER_ANDROID) {
        if (ctx != EGL_NO_CONTEXT) {
            setErrorReturn(EGL_BAD_CONTEXT, EGL_NO_IMAGE_KHR);
        }

        android_native_buffer_t* native_buffer = (android_native_buffer_t*)buffer;

        if (native_buffer->common.magic != ANDROID_NATIVE_BUFFER_MAGIC)
            setErrorReturn(EGL_BAD_PARAMETER, EGL_NO_IMAGE_KHR);

        if (native_buffer->common.version != sizeof(android_native_buffer_t))
            setErrorReturn(EGL_BAD_PARAMETER, EGL_NO_IMAGE_KHR);

        cb_handle_t *cb = (cb_handle_t *)(native_buffer->handle);

        switch (cb->format) {
            case HAL_PIXEL_FORMAT_RGBA_8888:
            case HAL_PIXEL_FORMAT_RGBX_8888:
            case HAL_PIXEL_FORMAT_RGB_888:
            case HAL_PIXEL_FORMAT_RGB_565:
            case HAL_PIXEL_FORMAT_BGRA_8888:
                break;
            default:
                setErrorReturn(EGL_BAD_PARAMETER, EGL_NO_IMAGE_KHR);
        }

        native_buffer->common.incRef(&native_buffer->common);

        EGLImage_t *image = new EGLImage_t();
        image->dpy = dpy;
        image->target = target;
        image->native_buffer = native_buffer;

        return (EGLImageKHR)image;
    }
    else if (target == EGL_GL_TEXTURE_2D_KHR) {
        VALIDATE_CONTEXT_RETURN(ctx, EGL_NO_IMAGE_KHR);

        EGLContext_t *context = static_cast<EGLContext_t*>(ctx);
        DEFINE_AND_VALIDATE_HOST_CONNECTION(EGL_NO_IMAGE_KHR);

        uint32_t ctxHandle = (context) ? context->rcContext : 0;
        GLuint texture = (GLuint)reinterpret_cast<uintptr_t>(buffer);
        uint32_t img = rcEnc->rcCreateClientImage(rcEnc, ctxHandle, target, texture);
        EGLImage_t *image = new EGLImage_t();
        image->dpy = dpy;
        image->target = target;
        image->host_egl_image = img;

        return (EGLImageKHR)image;
    }

    setErrorReturn(EGL_BAD_PARAMETER, EGL_NO_IMAGE_KHR);
}

EGLBoolean eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR img)
{
    VALIDATE_DISPLAY_INIT(dpy, EGL_FALSE);
    EGLImage_t *image = (EGLImage_t*)img;

    if (!image || image->dpy != dpy) {
        RETURN_ERROR(EGL_FALSE, EGL_BAD_PARAMETER);
    }

    if (image->target == EGL_NATIVE_BUFFER_ANDROID) {
        android_native_buffer_t* native_buffer = image->native_buffer;

        if (native_buffer->common.magic != ANDROID_NATIVE_BUFFER_MAGIC)
            setErrorReturn(EGL_BAD_PARAMETER, EGL_FALSE);

        if (native_buffer->common.version != sizeof(android_native_buffer_t))
            setErrorReturn(EGL_BAD_PARAMETER, EGL_FALSE);

        native_buffer->common.decRef(&native_buffer->common);
        delete image;

        return EGL_TRUE;
    }
    else if (image->target == EGL_GL_TEXTURE_2D_KHR) {
        uint32_t host_egl_image = image->host_egl_image;
        delete image;
        DEFINE_AND_VALIDATE_HOST_CONNECTION(EGL_FALSE);
        return rcEnc->rcDestroyClientImage(rcEnc, host_egl_image);
    }

    setErrorReturn(EGL_BAD_PARAMETER, EGL_FALSE);
}

#define FENCE_SYNC_HANDLE (EGLSyncKHR)0xFE4CE

EGLSyncKHR eglCreateSyncKHR(EGLDisplay dpy, EGLenum type,
        const EGLint *attrib_list)
{
    // TODO: This implementation could be faster. We should require the host EGL
    // to support KHR_fence_sync, or at least pipe the fence command to the host
    // and wait for it (probably involving a glFinish on the host) in
    // eglClientWaitSyncKHR.

    VALIDATE_DISPLAY(dpy, EGL_NO_SYNC_KHR);

    if (type != EGL_SYNC_FENCE_KHR ||
            (attrib_list != NULL && attrib_list[0] != EGL_NONE)) {
        setErrorReturn(EGL_BAD_ATTRIBUTE, EGL_NO_SYNC_KHR);
    }

    EGLThreadInfo *tInfo = getEGLThreadInfo();
    if (!tInfo || !tInfo->currentContext) {
        setErrorReturn(EGL_BAD_MATCH, EGL_NO_SYNC_KHR);
    }

    if (tInfo->currentContext->version == 2) {
        s_display.gles2_iface()->finish();
    } else {
        s_display.gles_iface()->finish();
    }

    return FENCE_SYNC_HANDLE;
}

EGLBoolean eglDestroySyncKHR(EGLDisplay dpy, EGLSyncKHR sync)
{
    (void)dpy;

    if (sync != FENCE_SYNC_HANDLE) {
        setErrorReturn(EGL_BAD_PARAMETER, EGL_FALSE);
    }

    return EGL_TRUE;
}

EGLint eglClientWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags,
        EGLTimeKHR timeout)
{
    (void)dpy;
    (void)flags;
    (void)timeout;

    if (sync != FENCE_SYNC_HANDLE) {
        setErrorReturn(EGL_BAD_PARAMETER, EGL_FALSE);
    }

    return EGL_CONDITION_SATISFIED_KHR;
}

EGLBoolean eglGetSyncAttribKHR(EGLDisplay dpy, EGLSyncKHR sync,
        EGLint attribute, EGLint *value)
{
    (void)dpy;

    if (sync != FENCE_SYNC_HANDLE) {
        setErrorReturn(EGL_BAD_PARAMETER, EGL_FALSE);
    }

    switch (attribute) {
    case EGL_SYNC_TYPE_KHR:
        *value = EGL_SYNC_FENCE_KHR;
        return EGL_TRUE;
    case EGL_SYNC_STATUS_KHR:
        *value = EGL_SIGNALED_KHR;
        return EGL_TRUE;
    case EGL_SYNC_CONDITION_KHR:
        *value = EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR;
        return EGL_TRUE;
    default:
        setErrorReturn(EGL_BAD_ATTRIBUTE, EGL_FALSE);
    }
}
