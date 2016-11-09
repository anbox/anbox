/*
* Copyright (C) 2011-2015 The Android Open Source Project
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

#include "FrameBuffer.h"

#include "DispatchTables.h"
#include "NativeSubWindow.h"
#include "RenderThreadInfo.h"
#include "TimeUtils.h"
#include "gles2_dec.h"

#include "OpenGLESDispatch/EGLDispatch.h"

#include "emugl/common/logging.h"

#include <stdio.h>

namespace {

// Helper class to call the bind_locked() / unbind_locked() properly.
class ScopedBind {
public:
    // Constructor will call bind_locked() on |fb|.
    // Use isValid() to check for errors.
    ScopedBind(FrameBuffer* fb) : mFb(fb) {
        if (!mFb->bind_locked()) {
            mFb = NULL;
        }
    }

    // Returns true if contruction bound the framebuffer context properly.
    bool isValid() const { return mFb != NULL; }

    // Unbound the framebuffer explictly. This is also called by the
    // destructor.
    void release() {
        if (mFb) {
            mFb->unbind_locked();
            mFb = NULL;
        }
    }

    // Destructor will call release().
    ~ScopedBind() {
        release();
    }

private:
    FrameBuffer* mFb;
};

// Implementation of a ColorBuffer::Helper instance that redirects calls
// to a FrameBuffer instance.
class ColorBufferHelper : public ColorBuffer::Helper {
public:
    ColorBufferHelper(FrameBuffer* fb) : mFb(fb) {}

    virtual bool setupContext() {
        return mFb->bind_locked();
    }

    virtual void teardownContext() {
        mFb->unbind_locked();
    }

    virtual TextureDraw* getTextureDraw() const {
        return mFb->getTextureDraw();
    }
private:
    FrameBuffer* mFb;
};

}  // namespace

FrameBuffer *FrameBuffer::s_theFrameBuffer = NULL;
HandleType FrameBuffer::s_nextHandle = 0;

static char* getGLES1ExtensionString(EGLDisplay p_dpy)
{
    EGLConfig config;
    EGLSurface surface;

    static const GLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
        EGL_NONE
    };

    int n;
    if (!s_egl.eglChooseConfig(p_dpy, configAttribs,
                               &config, 1, &n) || n == 0) {
        ERR("%s: Could not find GLES 1.x config!\n", __FUNCTION__);
        return NULL;
    }

    DBG("%s: Found config %p\n", __FUNCTION__, (void*)config);

    static const EGLint pbufAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };

    surface = s_egl.eglCreatePbufferSurface(p_dpy, config, pbufAttribs);
    if (surface == EGL_NO_SURFACE) {
        ERR("%s: Could not create GLES 1.x Pbuffer!\n", __FUNCTION__);
        return NULL;
    }

    static const GLint gles1ContextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 1,
        EGL_NONE
    };

    EGLContext ctx = s_egl.eglCreateContext(p_dpy, config,
                                            EGL_NO_CONTEXT,
                                            gles1ContextAttribs);
    if (ctx == EGL_NO_CONTEXT) {
        ERR("%s: Could not create GLES 1.x Context!\n", __FUNCTION__);
        s_egl.eglDestroySurface(p_dpy, surface);
        return NULL;
    }

    if (!s_egl.eglMakeCurrent(p_dpy, surface, surface, ctx)) {
        ERR("%s: Could not make GLES 1.x context current!\n", __FUNCTION__);
        s_egl.eglDestroySurface(p_dpy, surface);
        s_egl.eglDestroyContext(p_dpy, ctx);
        return NULL;
    }

    // the string pointer may become invalid when the context is destroyed
    const char* s = (const char*)s_gles1.glGetString(GL_EXTENSIONS);
    char* extString = strdup(s ? s : "");

    s_egl.eglMakeCurrent(p_dpy, NULL, NULL, NULL);
    s_egl.eglDestroyContext(p_dpy, ctx);
    s_egl.eglDestroySurface(p_dpy, surface);

    return extString;
}

void FrameBuffer::finalize(){
    m_colorbuffers.clear();
    m_windows.clear();
    m_contexts.clear();
    s_egl.eglMakeCurrent(m_eglDisplay, NULL, NULL, NULL);
    s_egl.eglDestroyContext(m_eglDisplay, m_eglContext);
    s_egl.eglDestroyContext(m_eglDisplay, m_pbufContext);
    s_egl.eglDestroySurface(m_eglDisplay, m_pbufSurface);
}

bool FrameBuffer::initialize(EGLNativeDisplayType nativeDisplay)
{
    GL_LOG("FrameBuffer::initialize");
    if (s_theFrameBuffer != NULL) {
        return true;
    }

    //
    // allocate space for the FrameBuffer object
    //
    FrameBuffer *fb = new FrameBuffer();
    if (!fb) {
        ERR("Failed to create fb\n");
        return false;
    }

    //
    // Initialize backend EGL display
    //
    fb->m_eglDisplay = s_egl.eglGetDisplay(nativeDisplay);
    if (fb->m_eglDisplay == EGL_NO_DISPLAY) {
        ERR("Failed to Initialize backend EGL display\n");
        delete fb;
        return false;
    }

    GL_LOG("call eglInitialize");
    if (!s_egl.eglInitialize(fb->m_eglDisplay,
                             &fb->m_caps.eglMajor,
                             &fb->m_caps.eglMinor)) {
        ERR("Failed to eglInitialize\n");
        GL_LOG("Failed to eglInitialize");
        delete fb;
        return false;
    }

    DBG("egl: %d %d\n", fb->m_caps.eglMajor, fb->m_caps.eglMinor);
    GL_LOG("egl: %d %d", fb->m_caps.eglMajor, fb->m_caps.eglMinor);
    s_egl.eglBindAPI(EGL_OPENGL_ES_API);

    //
    // if GLES2 plugin has loaded - try to make GLES2 context and
    // get GLES2 extension string
    //
    char* gles1Extensions = NULL;
    gles1Extensions = getGLES1ExtensionString(fb->m_eglDisplay);
    if (!gles1Extensions) {
        // Could not create GLES2 context - drop GL2 capability
        ERR("Failed to obtain GLES 2.x extensions string!\n");
        delete fb;
        return false;
    }

    //
    // Create EGL context for framebuffer post rendering.
    //
    GLint surfaceType = EGL_WINDOW_BIT| EGL_PBUFFER_BIT;
    const GLint configAttribs[] = {
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_SURFACE_TYPE, surfaceType,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    int n;
    if (!s_egl.eglChooseConfig(fb->m_eglDisplay, configAttribs,
                               &fb->m_eglConfig, 1, &n)) {
        ERR("Failed on eglChooseConfig\n");
        free(gles1Extensions);
        delete fb;
        return false;
    }

    static const GLint glContextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    GL_LOG("attempting to create egl context");
    fb->m_eglContext = s_egl.eglCreateContext(fb->m_eglDisplay,
                                              fb->m_eglConfig,
                                              EGL_NO_CONTEXT,
                                              glContextAttribs);
    if (fb->m_eglContext == EGL_NO_CONTEXT) {
        ERR("Failed to create context 0x%x\n", s_egl.eglGetError());
        free(gles1Extensions);
        delete fb;
        return false;
    }

    GL_LOG("attempting to create egl pbuffer context");
    //
    // Create another context which shares with the eglContext to be used
    // when we bind the pbuffer. That prevent switching drawable binding
    // back and forth on framebuffer context.
    // The main purpose of it is to solve a "blanking" behaviour we see on
    // on Mac platform when switching binded drawable for a context however
    // it is more efficient on other platforms as well.
    //
    fb->m_pbufContext = s_egl.eglCreateContext(fb->m_eglDisplay,
                                               fb->m_eglConfig,
                                               fb->m_eglContext,
                                               glContextAttribs);
    if (fb->m_pbufContext == EGL_NO_CONTEXT) {
        ERR("Failed to create Pbuffer Context 0x%x\n", s_egl.eglGetError());
        free(gles1Extensions);
        delete fb;
        return false;
    }

    GL_LOG("context creation successful");
    //
    // create a 1x1 pbuffer surface which will be used for binding
    // the FB context.
    // The FB output will go to a subwindow, if one exist.
    //
    static const EGLint pbufAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };

    fb->m_pbufSurface = s_egl.eglCreatePbufferSurface(fb->m_eglDisplay,
                                                      fb->m_eglConfig,
                                                      pbufAttribs);
    if (fb->m_pbufSurface == EGL_NO_SURFACE) {
        ERR("Failed to create pbuf surface for FB 0x%x\n", s_egl.eglGetError());
        free(gles1Extensions);
        delete fb;
        return false;
    }

    GL_LOG("attempting to make context current");
    // Make the context current
    ScopedBind bind(fb);
    if (!bind.isValid()) {
        ERR("Failed to make current\n");
        free(gles1Extensions);
        delete fb;
        return false;
    }
    GL_LOG("context-current successful");

    //
    // Initilize framebuffer capabilities
    //
    //const char* gles2Extensions = (const char *)s_gles2.glGetString(GL_EXTENSIONS);
    bool has_gl_oes_image = false;

//     printf("GLES1 [%s]\n", gles1Extensions);
//     printf("GLES2 [%s]\n", gles2Extensions);

    has_gl_oes_image = true;

    if (has_gl_oes_image) {
        has_gl_oes_image &= strstr(gles1Extensions, "GL_OES_EGL_image") != NULL;
    }
    free((void*)gles1Extensions);
    gles1Extensions = NULL;

    const char *eglExtensions = s_egl.eglQueryString(fb->m_eglDisplay,
                                                     EGL_EXTENSIONS);

    if (eglExtensions && has_gl_oes_image) {
        fb->m_caps.has_eglimage_texture_2d =
             strstr(eglExtensions, "EGL_KHR_gl_texture_2D_image") != NULL;
        fb->m_caps.has_eglimage_renderbuffer =
             strstr(eglExtensions, "EGL_KHR_gl_renderbuffer_image") != NULL;
    }
    else {
        fb->m_caps.has_eglimage_texture_2d = false;
        fb->m_caps.has_eglimage_renderbuffer = false;
    }

    //
    // Fail initialization if not all of the following extensions
    // exist:
    //     EGL_KHR_gl_texture_2d_image
    //     GL_OES_EGL_IMAGE (by both GLES implementations [1 and 2])
    //
    if (!fb->m_caps.has_eglimage_texture_2d) {
        ERR("Failed: Missing egl_image related extension(s)\n");
        bind.release();
        delete fb;
        return false;
    }

    GL_LOG("host system has enough extensions");
    //
    // Initialize set of configs
    //
    fb->m_configs = new FbConfigList(fb->m_eglDisplay);
    if (fb->m_configs->empty()) {
        ERR("Failed: Initialize set of configs\n");
        bind.release();
        delete fb;
        return false;
    }

    //
    // Check that we have config for each GLES and GLES2
    //
    size_t nConfigs = fb->m_configs->size();
    int nGLConfigs = 0;
    int nGL2Configs = 0;
    for (size_t i = 0; i < nConfigs; ++i) {
        GLint rtype = fb->m_configs->get(i)->getRenderableType();
        if (0 != (rtype & EGL_OPENGL_ES_BIT)) {
            nGLConfigs++;
        }
        if (0 != (rtype & EGL_OPENGL_ES2_BIT)) {
            nGL2Configs++;
        }
    }

    //
    // Fail initialization if no GLES configs exist
    //
    if (nGLConfigs == 0) {
        bind.release();
        delete fb;
        return false;
    }

    //
    // If no GLES2 configs exist - not GLES2 capability
    //
    if (nGL2Configs == 0) {
        ERR("Failed: No GLES 2.x configs found!\n");
        bind.release();
        delete fb;
        return false;
    }

    GL_LOG("There are sufficient EGLconfigs available");

    //
    // Cache the GL strings so we don't have to think about threading or
    // current-context when asked for them.
    //
    fb->m_glVendor = (const char*)s_gles2.glGetString(GL_VENDOR);
    fb->m_glRenderer = (const char*)s_gles2.glGetString(GL_RENDERER);
    fb->m_glVersion = (const char*)s_gles2.glGetString(GL_VERSION);

    fb->m_textureDraw = new TextureDraw(fb->m_eglDisplay);
    if (!fb->m_textureDraw) {
        ERR("Failed: creation of TextureDraw instance\n");
        bind.release();
        delete fb;
        return false;
    }

    // release the FB context
    bind.release();

    //
    // Keep the singleton framebuffer pointer
    //
    s_theFrameBuffer = fb;
    GL_LOG("basic EGL initialization successful");
    return true;
}

FrameBuffer::FrameBuffer() :
    m_configs(NULL),
    m_eglDisplay(EGL_NO_DISPLAY),
    m_colorBufferHelper(new ColorBufferHelper(this)),
    m_eglContext(EGL_NO_CONTEXT),
    m_pbufContext(EGL_NO_CONTEXT),
    m_prevContext(EGL_NO_CONTEXT),
    m_prevReadSurf(EGL_NO_SURFACE),
    m_prevDrawSurf(EGL_NO_SURFACE),
    m_textureDraw(NULL),
    m_lastPostedColorBuffer(0),
    m_statsNumFrames(0),
    m_statsStartTime(0LL),
    m_glVendor(NULL),
    m_glRenderer(NULL),
    m_glVersion(NULL)
{
    m_fpsStats = getenv("SHOW_FPS_STATS") != NULL;
}

FrameBuffer::~FrameBuffer() {
    delete m_textureDraw;
    delete m_configs;
    delete m_colorBufferHelper;
}

struct FrameBufferWindow {
    EGLNativeWindowType native_window = 0;
    EGLSurface surface = EGL_NO_SURFACE;
    bool needViewportUpdate = false;
    int width;
    int height;
};

FrameBufferWindow* FrameBuffer::createWindow(int x, int y, int width, int height) {
    m_lock.lock();

    auto native_window = createSubWindow(0, x, y, width, height);
    if (!native_window) {
        m_lock.unlock();
        return nullptr;
    }

    auto window = new FrameBufferWindow;
    window->native_window = native_window;
    window->width = width;
    window->height = height;

    window->surface = s_egl.eglCreateWindowSurface(
                m_eglDisplay, m_eglConfig, window->native_window, nullptr);
    if (window->surface == EGL_NO_SURFACE) {
        destroyWindow(window);
        m_lock.unlock();
        return nullptr;
    }

    if (!bindWindow_locked(window)) {
        destroyWindow(window);
        m_lock.unlock();
        return nullptr;
    }

    s_gles2.glViewport(0, 0, width, height);
    s_gles2.glClear(GL_COLOR_BUFFER_BIT |
                    GL_DEPTH_BUFFER_BIT |
                    GL_STENCIL_BUFFER_BIT);
    s_egl.eglSwapBuffers(m_eglDisplay, window->surface);

    unbind_locked();

    m_lock.unlock();

    return window;
}

void FrameBuffer::updateWindow(FrameBufferWindow *window, int x, int y, int width, int height) {
    if (!window)
        return;

    updateSubWindow(window->native_window, x, y, width, height);
    window->width = width;
    window->height = height;
    window->needViewportUpdate = true;
}

void FrameBuffer::destroyWindow(FrameBufferWindow *window) {
    if (!window)
        return;

    m_lock.lock();

    s_egl.eglMakeCurrent(m_eglDisplay, nullptr, nullptr, nullptr);

    if (window->surface != EGL_NO_SURFACE)
        s_egl.eglDestroySurface(m_eglDisplay, window->surface);

    destroySubWindow(window->native_window);

    delete window;

    m_lock.unlock();
}

HandleType FrameBuffer::genHandle()
{
    HandleType id;
    do {
        id = ++s_nextHandle;
    } while( id == 0 ||
             m_contexts.find(id) != m_contexts.end() ||
             m_windows.find(id) != m_windows.end() );

    return id;
}

HandleType FrameBuffer::createColorBuffer(int p_width, int p_height,
                                          GLenum p_internalFormat)
{
    emugl::Mutex::AutoLock mutex(m_lock);
    HandleType ret = 0;

    ColorBufferPtr cb(ColorBuffer::create(
            getDisplay(),
            p_width,
            p_height,
            p_internalFormat,
            getCaps().has_eglimage_texture_2d,
            m_colorBufferHelper));
    if (cb.Ptr() != NULL) {
        ret = genHandle();
        m_colorbuffers[ret].cb = cb;
        m_colorbuffers[ret].refcount = 1;
    }
    return ret;
}

HandleType FrameBuffer::createRenderContext(int p_config, HandleType p_share,
                                            bool p_isGL2)
{
    emugl::Mutex::AutoLock mutex(m_lock);
    HandleType ret = 0;

    const FbConfig* config = getConfigs()->get(p_config);
    if (!config) {
        return ret;
    }

    RenderContextPtr share(NULL);
    if (p_share != 0) {
        RenderContextMap::iterator s(m_contexts.find(p_share));
        if (s == m_contexts.end()) {
            return ret;
        }
        share = (*s).second;
    }
    EGLContext sharedContext =
            share.Ptr() ? share->getEGLContext() : EGL_NO_CONTEXT;

    RenderContextPtr rctx(RenderContext::create(
        m_eglDisplay, config->getEglConfig(), sharedContext, p_isGL2));
    if (rctx.Ptr() != NULL) {
        ret = genHandle();
        m_contexts[ret] = rctx;
        RenderThreadInfo *tinfo = RenderThreadInfo::get();
        tinfo->m_contextSet.insert(ret);
    }
    return ret;
}

HandleType FrameBuffer::createWindowSurface(int p_config, int p_width, int p_height)
{
    emugl::Mutex::AutoLock mutex(m_lock);

    HandleType ret = 0;

    const FbConfig* config = getConfigs()->get(p_config);
    if (!config) {
        return ret;
    }

    WindowSurfacePtr win(WindowSurface::create(
            getDisplay(), config->getEglConfig(), p_width, p_height));
    if (win.Ptr() != NULL) {
        ret = genHandle();
        m_windows[ret] = std::pair<WindowSurfacePtr, HandleType>(win,0);
        RenderThreadInfo *tinfo = RenderThreadInfo::get();
        tinfo->m_windowSet.insert(ret);
    }

    return ret;
}

void FrameBuffer::drainRenderContext()
{
    emugl::Mutex::AutoLock mutex(m_lock);
    RenderThreadInfo *tinfo = RenderThreadInfo::get();
    if (tinfo->m_contextSet.empty()) return;
    for (std::set<HandleType>::iterator it = tinfo->m_contextSet.begin();
            it != tinfo->m_contextSet.end(); ++it) {
        HandleType contextHandle = *it;
        m_contexts.erase(contextHandle);
    }
    tinfo->m_contextSet.clear();
}

void FrameBuffer::drainWindowSurface()
{
    emugl::Mutex::AutoLock mutex(m_lock);
    RenderThreadInfo *tinfo = RenderThreadInfo::get();
    if (tinfo->m_windowSet.empty()) return;
    for (std::set<HandleType>::iterator it = tinfo->m_windowSet.begin();
            it != tinfo->m_windowSet.end(); ++it) {
        HandleType windowHandle = *it;
        if (m_windows.find(windowHandle) != m_windows.end()) {
            HandleType oldColorBufferHandle = m_windows[windowHandle].second;
            if (oldColorBufferHandle) {
                ColorBufferMap::iterator cit(m_colorbuffers.find(oldColorBufferHandle));
                if (cit != m_colorbuffers.end()) {
                    if (--(*cit).second.refcount == 0) { m_colorbuffers.erase(cit); }
                }
            }
            m_windows.erase(windowHandle);
        }
    }
    tinfo->m_windowSet.clear();
}

void FrameBuffer::DestroyRenderContext(HandleType p_context)
{
    emugl::Mutex::AutoLock mutex(m_lock);
    m_contexts.erase(p_context);
    RenderThreadInfo *tinfo = RenderThreadInfo::get();
    if (tinfo->m_contextSet.empty()) return;
    tinfo->m_contextSet.erase(p_context);
}

void FrameBuffer::DestroyWindowSurface(HandleType p_surface)
{
    emugl::Mutex::AutoLock mutex(m_lock);
    if (m_windows.find(p_surface) != m_windows.end()) {
        m_windows.erase(p_surface);
        RenderThreadInfo *tinfo = RenderThreadInfo::get();
        if (tinfo->m_windowSet.empty()) return;
        tinfo->m_windowSet.erase(p_surface);
    }
}

int FrameBuffer::openColorBuffer(HandleType p_colorbuffer)
{
    emugl::Mutex::AutoLock mutex(m_lock);
    ColorBufferMap::iterator c(m_colorbuffers.find(p_colorbuffer));
    if (c == m_colorbuffers.end()) {
        // bad colorbuffer handle
        ERR("FB: openColorBuffer cb handle %#x not found\n", p_colorbuffer);
        return -1;
    }
    (*c).second.refcount++;
    return 0;
}

void FrameBuffer::closeColorBuffer(HandleType p_colorbuffer)
{
    emugl::Mutex::AutoLock mutex(m_lock);
    ColorBufferMap::iterator c(m_colorbuffers.find(p_colorbuffer));
    if (c == m_colorbuffers.end()) {
        // This is harmless: it is normal for guest system to issue
        // closeColorBuffer command when the color buffer is already
        // garbage collected on the host. (we dont have a mechanism
        // to give guest a notice yet)
        return;
    }
    if (--(*c).second.refcount == 0) {
        m_colorbuffers.erase(c);
    }
}

bool FrameBuffer::flushWindowSurfaceColorBuffer(HandleType p_surface)
{
    emugl::Mutex::AutoLock mutex(m_lock);

    WindowSurfaceMap::iterator w( m_windows.find(p_surface) );
    if (w == m_windows.end()) {
        ERR("FB::flushWindowSurfaceColorBuffer: window handle %#x not found\n", p_surface);
        // bad surface handle
        return false;
    }

    WindowSurface* surface = (*w).second.first.Ptr();
    surface->flushColorBuffer();

    return true;
}

bool FrameBuffer::setWindowSurfaceColorBuffer(HandleType p_surface,
                                              HandleType p_colorbuffer)
{
    emugl::Mutex::AutoLock mutex(m_lock);

    WindowSurfaceMap::iterator w( m_windows.find(p_surface) );
    if (w == m_windows.end()) {
        // bad surface handle
        ERR("%s: bad window surface handle %#x\n", __FUNCTION__, p_surface);
        return false;
    }

    ColorBufferMap::iterator c( m_colorbuffers.find(p_colorbuffer) );
    if (c == m_colorbuffers.end()) {
        DBG("%s: bad color buffer handle %#x\n", __FUNCTION__, p_colorbuffer);
        // bad colorbuffer handle
        return false;
    }

    (*w).second.first->setColorBuffer((*c).second.cb);
    (*w).second.second = p_colorbuffer;
    return true;
}

void FrameBuffer::readColorBuffer(HandleType p_colorbuffer,
                                    int x, int y, int width, int height,
                                    GLenum format, GLenum type, void *pixels)
{
    emugl::Mutex::AutoLock mutex(m_lock);

    ColorBufferMap::iterator c( m_colorbuffers.find(p_colorbuffer) );
    if (c == m_colorbuffers.end()) {
        // bad colorbuffer handle
        return;
    }

    (*c).second.cb->readPixels(x, y, width, height, format, type, pixels);
}

bool FrameBuffer::updateColorBuffer(HandleType p_colorbuffer,
                                    int x, int y, int width, int height,
                                    GLenum format, GLenum type, void *pixels)
{
    emugl::Mutex::AutoLock mutex(m_lock);

    ColorBufferMap::iterator c( m_colorbuffers.find(p_colorbuffer) );
    if (c == m_colorbuffers.end()) {
        // bad colorbuffer handle
        return false;
    }

    (*c).second.cb->subUpdate(x, y, width, height, format, type, pixels);

    return true;
}

bool FrameBuffer::bindColorBufferToTexture(HandleType p_colorbuffer)
{
    emugl::Mutex::AutoLock mutex(m_lock);

    ColorBufferMap::iterator c( m_colorbuffers.find(p_colorbuffer) );
    if (c == m_colorbuffers.end()) {
        // bad colorbuffer handle
        return false;
    }

    return (*c).second.cb->bindToTexture();
}

bool FrameBuffer::bindColorBufferToRenderbuffer(HandleType p_colorbuffer)
{
    emugl::Mutex::AutoLock mutex(m_lock);

    ColorBufferMap::iterator c( m_colorbuffers.find(p_colorbuffer) );
    if (c == m_colorbuffers.end()) {
        // bad colorbuffer handle
        return false;
    }

    return (*c).second.cb->bindToRenderbuffer();
}

bool FrameBuffer::bindContext(HandleType p_context,
                              HandleType p_drawSurface,
                              HandleType p_readSurface)
{
    emugl::Mutex::AutoLock mutex(m_lock);

    WindowSurfacePtr draw(NULL), read(NULL);
    RenderContextPtr ctx(NULL);

    //
    // if this is not an unbind operation - make sure all handles are good
    //
    if (p_context || p_drawSurface || p_readSurface) {
        RenderContextMap::iterator r( m_contexts.find(p_context) );
        if (r == m_contexts.end()) {
            // bad context handle
            return false;
        }

        ctx = (*r).second;
        WindowSurfaceMap::iterator w( m_windows.find(p_drawSurface) );
        if (w == m_windows.end()) {
            // bad surface handle
            return false;
        }
        draw = (*w).second.first;

        if (p_readSurface != p_drawSurface) {
            WindowSurfaceMap::iterator w( m_windows.find(p_readSurface) );
            if (w == m_windows.end()) {
                // bad surface handle
                return false;
            }
            read = (*w).second.first;
        }
        else {
            read = draw;
        }
    }

    if (!s_egl.eglMakeCurrent(m_eglDisplay,
                              draw ? draw->getEGLSurface() : EGL_NO_SURFACE,
                              read ? read->getEGLSurface() : EGL_NO_SURFACE,
                              ctx ? ctx->getEGLContext() : EGL_NO_CONTEXT)) {
        ERR("eglMakeCurrent failed\n");
        return false;
    }

    //
    // Bind the surface(s) to the context
    //
    RenderThreadInfo *tinfo = RenderThreadInfo::get();
    WindowSurfacePtr bindDraw, bindRead;
    if (draw.Ptr() == NULL && read.Ptr() == NULL) {
        // Unbind the current read and draw surfaces from the context
        bindDraw = tinfo->currDrawSurf;
        bindRead = tinfo->currReadSurf;
    } else {
        bindDraw = draw;
        bindRead = read;
    }

    if (bindDraw.Ptr() != NULL && bindRead.Ptr() != NULL) {
        if (bindDraw.Ptr() != bindRead.Ptr()) {
            bindDraw->bind(ctx, WindowSurface::BIND_DRAW);
            bindRead->bind(ctx, WindowSurface::BIND_READ);
        }
        else {
            bindDraw->bind(ctx, WindowSurface::BIND_READDRAW);
        }
    }

    //
    // update thread info with current bound context
    //
    tinfo->currContext = ctx;
    tinfo->currDrawSurf = draw;
    tinfo->currReadSurf = read;
    if (ctx) {
        if (ctx->isGL2()) tinfo->m_gl2Dec.setContextData(&ctx->decoderContextData());
        else tinfo->m_glDec.setContextData(&ctx->decoderContextData());
    }
    else {
        tinfo->m_glDec.setContextData(NULL);
        tinfo->m_gl2Dec.setContextData(NULL);
    }
    return true;
}

HandleType FrameBuffer::createClientImage(HandleType context, EGLenum target, GLuint buffer)
{
    RenderContextPtr ctx(NULL);

    if (context) {
        RenderContextMap::iterator r( m_contexts.find(context) );
        if (r == m_contexts.end()) {
            // bad context handle
            return false;
        }

        ctx = (*r).second;
    }

    EGLContext eglContext = ctx ? ctx->getEGLContext() : EGL_NO_CONTEXT;
    EGLImageKHR image = s_egl.eglCreateImageKHR(
                            m_eglDisplay, eglContext, target,
                            reinterpret_cast<EGLClientBuffer>(buffer), NULL);

    return (HandleType)reinterpret_cast<uintptr_t>(image);
}

EGLBoolean FrameBuffer::destroyClientImage(HandleType image)
{
    return s_egl.eglDestroyImageKHR(m_eglDisplay,
                                    reinterpret_cast<EGLImageKHR>(image));
}

//
// The framebuffer lock should be held when calling this function !
//
bool FrameBuffer::bind_locked()
{
    EGLContext prevContext = s_egl.eglGetCurrentContext();
    EGLSurface prevReadSurf = s_egl.eglGetCurrentSurface(EGL_READ);
    EGLSurface prevDrawSurf = s_egl.eglGetCurrentSurface(EGL_DRAW);

    if (!s_egl.eglMakeCurrent(m_eglDisplay, m_pbufSurface,
                              m_pbufSurface, m_pbufContext)) {
        ERR("eglMakeCurrent failed\n");
        return false;
    }

    m_prevContext = prevContext;
    m_prevReadSurf = prevReadSurf;
    m_prevDrawSurf = prevDrawSurf;
    return true;
}

bool FrameBuffer::bindWindow_locked(FrameBufferWindow *window)
{
    EGLContext prevContext = s_egl.eglGetCurrentContext();
    EGLSurface prevReadSurf = s_egl.eglGetCurrentSurface(EGL_READ);
    EGLSurface prevDrawSurf = s_egl.eglGetCurrentSurface(EGL_DRAW);

    if (!s_egl.eglMakeCurrent(m_eglDisplay, window->surface,
                              window->surface, m_eglContext)) {
        ERR("eglMakeCurrent failed\n");
        return false;
    }

    m_prevContext = prevContext;
    m_prevReadSurf = prevReadSurf;
    m_prevDrawSurf = prevDrawSurf;
    return true;
}

bool FrameBuffer::unbind_locked()
{
    if (!s_egl.eglMakeCurrent(m_eglDisplay, m_prevDrawSurf,
                              m_prevReadSurf, m_prevContext)) {
        return false;
    }

    m_prevContext = EGL_NO_CONTEXT;
    m_prevReadSurf = EGL_NO_SURFACE;
    m_prevDrawSurf = EGL_NO_SURFACE;
    return true;
}

bool FrameBuffer::post(FrameBufferWindow *window, HandleType p_colorbuffer, bool needLock)
{
    if (!window)
        return false;

    printf("%s window %p\n", __func__, window);

    if (needLock)
        m_lock.lock();

    bool ret;

    ColorBufferMap::iterator c( m_colorbuffers.find(p_colorbuffer) );
    if (c == m_colorbuffers.end())
        goto EXIT;

    m_lastPostedColorBuffer = p_colorbuffer;

    if (!bindWindow_locked(window))
        goto EXIT;

    if (window->needViewportUpdate) {
        s_gles2.glViewport(0, 0, window->width, window->height);
        window->needViewportUpdate = false;
    }

    s_gles2.glClearColor(0.0, 0.0, 1.0, 0.0);
    s_gles2.glClear(GL_COLOR_BUFFER_BIT);

    ret = (*c).second.cb->post(0.0f, 0, 0);
    if (ret) {
        s_egl.eglSwapBuffers(m_eglDisplay, window->surface);
    }

    // restore previous binding
    unbind_locked();

    //
    // output FPS statistics
    //
    if (m_fpsStats) {
        long long currTime = GetCurrentTimeMS();
        m_statsNumFrames++;
        if (currTime - m_statsStartTime >= 1000) {
            float dt = (float)(currTime - m_statsStartTime) / 1000.0f;
            printf("FPS: %5.3f\n", (float)m_statsNumFrames / dt);
            m_statsStartTime = currTime;
            m_statsNumFrames = 0;
        }
    }

EXIT:
    if (!ret)
        printf("post: FAILED\n");

    if (needLock) {
        m_lock.unlock();
    }
    return ret;
}
