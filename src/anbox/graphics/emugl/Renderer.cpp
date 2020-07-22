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
#define GLM_ENABLE_EXPERIMENTAL
#include "anbox/graphics/emugl/Renderer.h"
#include "anbox/graphics/emugl/DispatchTables.h"
#include "anbox/graphics/emugl/RenderThreadInfo.h"
#include "anbox/graphics/emugl/TimeUtils.h"
#include "anbox/graphics/gl_extensions.h"
#include "anbox/logger.h"

#include "external/android-emugl/host/include/OpenGLESDispatch/EGLDispatch.h"

// Generated with emugl at build time
#include "gles2_dec.h"

#include <stdio.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

namespace {

// Helper class to call the bind_locked() / unbind_locked() properly.
class ScopedBind {
 public:
  // Constructor will call bind_locked() on |fb|.
  // Use isValid() to check for errors.
  ScopedBind(Renderer *fb) : mFb(fb) {
    if (!fb->bind_locked()) {
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
  ~ScopedBind() { release(); }

 private:
  Renderer *mFb;
};

// Implementation of a ColorBuffer::Helper instance that redirects calls
// to a FrameBuffer instance.
class ColorBufferHelper : public ColorBuffer::Helper {
 public:
  ColorBufferHelper(Renderer *fb) : mFb(fb) {}

  virtual bool setupContext() { return mFb->bind_locked(); }

  virtual void teardownContext() { mFb->unbind_locked(); }

  virtual TextureDraw *getTextureDraw() const { return mFb->getTextureDraw(); }

 private:
  Renderer *mFb;
};
}  // namespace

HandleType Renderer::s_nextHandle = 0;

void Renderer::finalize() {
  m_colorbuffers.clear();
  m_windows.clear();
  m_contexts.clear();
  s_egl.eglMakeCurrent(m_eglDisplay, NULL, NULL, NULL);
  s_egl.eglDestroyContext(m_eglDisplay, m_eglContext);
  s_egl.eglDestroyContext(m_eglDisplay, m_pbufContext);
  s_egl.eglDestroySurface(m_eglDisplay, m_pbufSurface);
}

bool Renderer::initialize(EGLNativeDisplayType nativeDisplay) {
  m_eglDisplay = s_egl.eglGetDisplay(nativeDisplay);
  if (m_eglDisplay == EGL_NO_DISPLAY) {
    ERROR("Failed to Initialize backend EGL display");
    return false;
  }

  if (!s_egl.eglInitialize(m_eglDisplay, &m_caps.eglMajor, &m_caps.eglMinor)) {
    ERROR("Failed to initialize EGL");
    return false;
  }

  anbox::graphics::GLExtensions egl_extensions{s_egl.eglQueryString(m_eglDisplay, EGL_EXTENSIONS)};

  const auto surfaceless_supported = egl_extensions.support("EGL_KHR_surfaceless_context");
  if (!surfaceless_supported)
    DEBUG("EGL doesn't support surfaceless context");

  s_egl.eglBindAPI(EGL_OPENGL_ES_API);

  // Create EGL context for framebuffer post rendering.
  GLint surfaceType = EGL_WINDOW_BIT | EGL_PBUFFER_BIT;
  const GLint configAttribs[] = {EGL_RED_SIZE, 8,
                                 EGL_GREEN_SIZE, 8,
                                 EGL_BLUE_SIZE, 8,
                                 EGL_SURFACE_TYPE, surfaceType,
                                 EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                                 EGL_NONE};

  int n;
  if ((s_egl.eglChooseConfig(m_eglDisplay, configAttribs, &m_eglConfig,
                             1, &n) == EGL_FALSE) || n == 0) {
    ERROR("Failed to select EGL configuration");
    return false;
  }

  static const GLint glContextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2,
                                           EGL_NONE};

  m_eglContext = s_egl.eglCreateContext(m_eglDisplay, m_eglConfig,
                                        EGL_NO_CONTEXT, glContextAttribs);
  if (m_eglContext == EGL_NO_CONTEXT) {
    ERROR("Failed to create context: error=0x%x", s_egl.eglGetError());
    return false;
  }

  // Create another context which shares with the eglContext to be used
  // when we bind the pbuffer. That prevent switching drawable binding
  // back and forth on framebuffer context.
  // The main purpose of it is to solve a "blanking" behaviour we see on
  // on Mac platform when switching binded drawable for a context however
  // it is more efficient on other platforms as well.
  m_pbufContext = s_egl.eglCreateContext(m_eglDisplay, m_eglConfig, m_eglContext, glContextAttribs);
  if (m_pbufContext == EGL_NO_CONTEXT) {
    ERROR("Failed to create pbuffer context: error=0x%x", s_egl.eglGetError());
    return false;
  }

  if (!surfaceless_supported) {
    // Create a 1x1 pbuffer surface which will be used for binding
    // the FB context. The FB output will go to a subwindow, if one exist.
    static const EGLint pbufAttribs[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};

    m_pbufSurface = s_egl.eglCreatePbufferSurface(m_eglDisplay, m_eglConfig, pbufAttribs);
    if (m_pbufSurface == EGL_NO_SURFACE) {
      ERROR("Failed to create pbuffer surface: error=0x%x", s_egl.eglGetError());
      return false;
    }
  } else {
    DEBUG("Using a surfaceless EGL context");
    m_pbufSurface = EGL_NO_SURFACE;
  }

  // Make the context current
  ScopedBind bind(this);
  if (!bind.isValid()) {
    ERROR("Failed to make current");
    return false;
  }

  anbox::graphics::GLExtensions gl_extensions{reinterpret_cast<const char *>(s_gles2.glGetString(GL_EXTENSIONS))};
  if (gl_extensions.support("GL_OES_EGL_image")) {
    m_caps.has_eglimage_texture_2d = egl_extensions.support("EGL_KHR_gl_texture_2D_image");
    m_caps.has_eglimage_renderbuffer = egl_extensions.support("EGL_KHR_gl_renderbuffer_image");
  } else {
    m_caps.has_eglimage_texture_2d = false;
    m_caps.has_eglimage_renderbuffer = false;
  }

  // Fail initialization if not all of the following extensions
  // exist:
  //     EGL_KHR_gl_texture_2d_image
  //     GL_OES_EGL_IMAGE
  if (!m_caps.has_eglimage_texture_2d) {
    ERROR("Failed: Missing egl_image related extension(s)");
    bind.release();
    return false;
  }

  // Initialize set of configs
  m_configs = new RendererConfigList(m_eglDisplay);
  if (m_configs->empty()) {
    ERROR("Failed: Initialize set of configs");
    bind.release();
    return false;
  }

  // Check that we have config for each GLES and GLES2
  size_t nConfigs = m_configs->size();
  int nGLConfigs = 0;
  int nGL2Configs = 0;
  for (size_t i = 0; i < nConfigs; ++i) {
    GLint rtype = m_configs->get(i)->getRenderableType();
    if (0 != (rtype & EGL_OPENGL_ES_BIT)) {
      nGLConfigs++;
    }
    if (0 != (rtype & EGL_OPENGL_ES2_BIT)) {
      nGL2Configs++;
    }
  }

  // Fail initialization if no GLES configs exist
  if (nGLConfigs == 0) {
    bind.release();
    return false;
  }

  // If no GLES2 configs exist - not GLES2 capability
  if (nGL2Configs == 0) {
    ERROR("Failed: No GLES 2.x configs found!");
    bind.release();
    return false;
  }

  // Cache the GL strings so we don't have to think about threading or
  // current-context when asked for them.
  m_glVendor = reinterpret_cast<const char *>(s_gles2.glGetString(GL_VENDOR));
  m_glRenderer = reinterpret_cast<const char *>(s_gles2.glGetString(GL_RENDERER));
  m_glVersion = reinterpret_cast<const char *>(s_gles2.glGetString(GL_VERSION));

  m_textureDraw = new TextureDraw(m_eglDisplay);
  if (!m_textureDraw) {
    ERROR("Failed: creation of TextureDraw instance");
    bind.release();
    return false;
  }

  m_defaultProgram = m_family.add_program(vshader, defaultFShader);
  m_alphaProgram = m_family.add_program(vshader, alphaFShader);

  bind.release();

  DEBUG("Successfully initialized EGL");

  return true;
}

Renderer::Program::Program(GLuint program_id) {
  id = program_id;
  position_attr = s_gles2.glGetAttribLocation(id, "position");
  texcoord_attr = s_gles2.glGetAttribLocation(id, "texcoord");
  tex_uniform = s_gles2.glGetUniformLocation(id, "tex");
  center_uniform = s_gles2.glGetUniformLocation(id, "center");
  display_transform_uniform =
      s_gles2.glGetUniformLocation(id, "display_transform");
  transform_uniform = s_gles2.glGetUniformLocation(id, "transform");
  screen_to_gl_coords_uniform =
      s_gles2.glGetUniformLocation(id, "screen_to_gl_coords");
  alpha_uniform = s_gles2.glGetUniformLocation(id, "alpha");
}

Renderer::Renderer()
    : m_configs(NULL),
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
      m_glVersion(NULL) {
  m_fpsStats = getenv("SHOW_FPS_STATS") != NULL;
}

Renderer::~Renderer() {
  delete m_textureDraw;
  delete m_configs;
  delete m_colorBufferHelper;
}

struct RendererWindow {
  EGLNativeWindowType native_window = 0;
  EGLSurface surface = EGL_NO_SURFACE;
  anbox::graphics::Rect viewport;
  glm::mat4 screen_to_gl_coords = glm::mat4(1.0f);
  glm::mat4 display_transform = glm::mat4(1.0f);
};

RendererWindow *Renderer::createNativeWindow(
    EGLNativeWindowType native_window) {
  m_lock.lock();

  auto window = new RendererWindow;
  window->native_window = native_window;
  window->surface = s_egl.eglCreateWindowSurface(
      m_eglDisplay, m_eglConfig, window->native_window, nullptr);
  if (window->surface == EGL_NO_SURFACE) {
    delete window;
    m_lock.unlock();
    return nullptr;
  }

  if (!bindWindow_locked(window)) {
    s_egl.eglDestroySurface(m_eglDisplay, window->surface);
    delete window;
    m_lock.unlock();
    return nullptr;
  }

  s_gles2.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                  GL_STENCIL_BUFFER_BIT);
  s_egl.eglSwapBuffers(m_eglDisplay, window->surface);

  unbind_locked();

  m_nativeWindows.insert({native_window, window});

  m_lock.unlock();

  return window;
}

void Renderer::destroyNativeWindow(EGLNativeWindowType native_window) {
  auto w = m_nativeWindows.find(native_window);
  if (w == m_nativeWindows.end()) return;

  m_lock.lock();

  s_egl.eglMakeCurrent(m_eglDisplay, nullptr, nullptr, nullptr);

  if (w->second->surface != EGL_NO_SURFACE)
    s_egl.eglDestroySurface(m_eglDisplay, w->second->surface);

  delete w->second;
  m_nativeWindows.erase(w);

  m_lock.unlock();
}

HandleType Renderer::genHandle() {
  HandleType id;
  do {
    id = ++s_nextHandle;
  } while (id == 0 || m_contexts.find(id) != m_contexts.end() ||
           m_windows.find(id) != m_windows.end());

  return id;
}

HandleType Renderer::createColorBuffer(int p_width, int p_height,
                                       GLenum p_internalFormat) {
  std::unique_lock<std::mutex> l(m_lock);

  HandleType ret = 0;

  ColorBufferPtr cb(ColorBuffer::create(
      getDisplay(), p_width, p_height, p_internalFormat,
      getCaps().has_eglimage_texture_2d, m_colorBufferHelper));
  if (cb) {
    ret = genHandle();
    m_colorbuffers[ret].cb = cb;
    m_colorbuffers[ret].refcount = 1;
  }
  return ret;
}

HandleType Renderer::createRenderContext(int p_config, HandleType p_share,
                                         bool p_isGL2) {
  std::unique_lock<std::mutex> l(m_lock);

  HandleType ret = 0;

  const RendererConfig *config = getConfigs()->get(p_config);
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
      share ? share->getEGLContext() : EGL_NO_CONTEXT;

  RenderContextPtr rctx(RenderContext::create(
      m_eglDisplay, config->getEglConfig(), sharedContext, p_isGL2));
  if (rctx) {
    ret = genHandle();
    m_contexts[ret] = rctx;
    RenderThreadInfo *tinfo = RenderThreadInfo::get();
    tinfo->m_contextSet.insert(ret);
  }
  return ret;
}

HandleType Renderer::createWindowSurface(int p_config, int p_width,
                                         int p_height) {
  std::unique_lock<std::mutex> l(m_lock);

  HandleType ret = 0;

  const RendererConfig *config = getConfigs()->get(p_config);
  if (!config) {
    return ret;
  }

  WindowSurfacePtr win(WindowSurface::create(
      getDisplay(), config->getEglConfig(), p_width, p_height));
  if (win) {
    ret = genHandle();
    m_windows[ret] = std::pair<WindowSurfacePtr, HandleType>(win, 0);
    RenderThreadInfo *tinfo = RenderThreadInfo::get();
    tinfo->m_windowSet.insert(ret);
  }

  return ret;
}

void Renderer::drainRenderContext() {
  std::unique_lock<std::mutex> l(m_lock);

  RenderThreadInfo *tinfo = RenderThreadInfo::get();
  if (tinfo->m_contextSet.empty()) return;
  for (std::set<HandleType>::iterator it = tinfo->m_contextSet.begin();
       it != tinfo->m_contextSet.end(); ++it) {
    HandleType contextHandle = *it;
    m_contexts.erase(contextHandle);
  }
  tinfo->m_contextSet.clear();
}

void Renderer::drainWindowSurface() {
  std::unique_lock<std::mutex> l(m_lock);

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
          if (--(*cit).second.refcount == 0) {
            m_colorbuffers.erase(cit);
          }
        }
      }
      m_windows.erase(windowHandle);
    }
  }
  tinfo->m_windowSet.clear();
}

void Renderer::DestroyRenderContext(HandleType p_context) {
  std::unique_lock<std::mutex> l(m_lock);

  m_contexts.erase(p_context);
  RenderThreadInfo *tinfo = RenderThreadInfo::get();
  if (tinfo->m_contextSet.empty()) return;
  tinfo->m_contextSet.erase(p_context);
}

void Renderer::DestroyWindowSurface(HandleType p_surface) {
  std::unique_lock<std::mutex> l(m_lock);

  if (m_windows.find(p_surface) != m_windows.end()) {
    m_windows.erase(p_surface);
    RenderThreadInfo *tinfo = RenderThreadInfo::get();
    if (tinfo->m_windowSet.empty()) return;
    tinfo->m_windowSet.erase(p_surface);
  }
}

int Renderer::openColorBuffer(HandleType p_colorbuffer) {
  std::unique_lock<std::mutex> l(m_lock);

  ColorBufferMap::iterator c(m_colorbuffers.find(p_colorbuffer));
  if (c == m_colorbuffers.end()) {
    // bad colorbuffer handle
    ERROR("FB: openColorBuffer cb handle %#x not found", p_colorbuffer);
    return -1;
  }
  (*c).second.refcount++;
  return 0;
}

void Renderer::closeColorBuffer(HandleType p_colorbuffer) {
  std::unique_lock<std::mutex> l(m_lock);

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

bool Renderer::flushWindowSurfaceColorBuffer(HandleType p_surface) {
  std::unique_lock<std::mutex> l(m_lock);

  WindowSurfaceMap::iterator w(m_windows.find(p_surface));
  if (w == m_windows.end()) {
    ERROR("FB::flushWindowSurfaceColorBuffer: window handle %#x not found",
        p_surface);
    // bad surface handle
    return false;
  }

  auto surface = (*w).second.first;
  if (!surface)
    return false;

  surface->flushColorBuffer();

  return true;
}

bool Renderer::setWindowSurfaceColorBuffer(HandleType p_surface,
                                           HandleType p_colorbuffer) {
  std::unique_lock<std::mutex> l(m_lock);

  WindowSurfaceMap::iterator w(m_windows.find(p_surface));
  if (w == m_windows.end()) {
    // bad surface handle
    ERROR("%s: bad window surface handle %#x", __FUNCTION__, p_surface);
    return false;
  }

  ColorBufferMap::iterator c(m_colorbuffers.find(p_colorbuffer));
  if (c == m_colorbuffers.end()) {
    DEBUG("%s: bad color buffer handle %#x", __FUNCTION__, p_colorbuffer);
    // bad colorbuffer handle
    return false;
  }

  (*w).second.first->setColorBuffer((*c).second.cb);
  (*w).second.second = p_colorbuffer;
  return true;
}

void Renderer::readColorBuffer(HandleType p_colorbuffer, int x, int y,
                               int width, int height, GLenum format,
                               GLenum type, void *pixels) {
  std::unique_lock<std::mutex> l(m_lock);

  ColorBufferMap::iterator c(m_colorbuffers.find(p_colorbuffer));
  if (c == m_colorbuffers.end()) {
    // bad colorbuffer handle
    return;
  }

  (*c).second.cb->readPixels(x, y, width, height, format, type, pixels);
}

bool Renderer::updateColorBuffer(HandleType p_colorbuffer, int x, int y,
                                 int width, int height, GLenum format,
                                 GLenum type, void *pixels) {
  std::unique_lock<std::mutex> l(m_lock);

  ColorBufferMap::iterator c(m_colorbuffers.find(p_colorbuffer));
  if (c == m_colorbuffers.end()) {
    // bad colorbuffer handle
    return false;
  }

  (*c).second.cb->subUpdate(x, y, width, height, format, type, pixels);

  return true;
}

bool Renderer::bindColorBufferToTexture(HandleType p_colorbuffer) {
  std::unique_lock<std::mutex> l(m_lock);

  ColorBufferMap::iterator c(m_colorbuffers.find(p_colorbuffer));
  if (c == m_colorbuffers.end()) {
    // bad colorbuffer handle
    return false;
  }

  return (*c).second.cb->bindToTexture();
}

bool Renderer::bindColorBufferToRenderbuffer(HandleType p_colorbuffer) {
  std::unique_lock<std::mutex> l(m_lock);

  ColorBufferMap::iterator c(m_colorbuffers.find(p_colorbuffer));
  if (c == m_colorbuffers.end()) {
    // bad colorbuffer handle
    return false;
  }

  return (*c).second.cb->bindToRenderbuffer();
}

bool Renderer::bindContext(HandleType p_context, HandleType p_drawSurface,
                           HandleType p_readSurface) {
  std::unique_lock<std::mutex> l(m_lock);

  WindowSurfacePtr draw(NULL), read(NULL);
  RenderContextPtr ctx(NULL);

  //
  // if this is not an unbind operation - make sure all handles are good
  //
  if (p_context || p_drawSurface || p_readSurface) {
    RenderContextMap::iterator r(m_contexts.find(p_context));
    if (r == m_contexts.end()) {
      // bad context handle
      return false;
    }

    ctx = (*r).second;
    WindowSurfaceMap::iterator w(m_windows.find(p_drawSurface));
    if (w == m_windows.end()) {
      // bad surface handle
      return false;
    }
    draw = (*w).second.first;

    if (p_readSurface != p_drawSurface) {
      WindowSurfaceMap::iterator w(m_windows.find(p_readSurface));
      if (w == m_windows.end()) {
        // bad surface handle
        return false;
      }
      read = (*w).second.first;
    } else {
      read = draw;
    }
  }

  if (!s_egl.eglMakeCurrent(m_eglDisplay,
                            draw ? draw->getEGLSurface() : EGL_NO_SURFACE,
                            read ? read->getEGLSurface() : EGL_NO_SURFACE,
                            ctx ? ctx->getEGLContext() : EGL_NO_CONTEXT)) {
    ERROR("eglMakeCurrent failed: 0x%04x", s_egl.eglGetError());
    return false;
  }

  //
  // Bind the surface(s) to the context
  //
  RenderThreadInfo *tinfo = RenderThreadInfo::get();
  WindowSurfacePtr bindDraw, bindRead;
  if (!draw && !read) {
    // Unbind the current read and draw surfaces from the context
    bindDraw = tinfo->currDrawSurf;
    bindRead = tinfo->currReadSurf;
  } else {
    bindDraw = draw;
    bindRead = read;
  }

  if (bindDraw && bindRead) {
    if (bindDraw != bindRead) {
      bindDraw->bind(ctx, WindowSurface::BIND_DRAW);
      bindRead->bind(ctx, WindowSurface::BIND_READ);
    } else {
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
    if (ctx->isGL2())
      tinfo->m_gl2Dec.setContextData(&ctx->decoderContextData());
    else
      tinfo->m_glDec.setContextData(&ctx->decoderContextData());
  } else {
    tinfo->m_glDec.setContextData(NULL);
    tinfo->m_gl2Dec.setContextData(NULL);
  }
  return true;
}

HandleType Renderer::createClientImage(HandleType context, EGLenum target,
                                       GLuint buffer) {
  RenderContextPtr ctx(NULL);

  if (context) {
    RenderContextMap::iterator r(m_contexts.find(context));
    if (r == m_contexts.end()) {
      // bad context handle
      return false;
    }

    ctx = (*r).second;
  }

  EGLContext eglContext = ctx ? ctx->getEGLContext() : EGL_NO_CONTEXT;
  EGLImageKHR image =
      s_egl.eglCreateImageKHR(m_eglDisplay, eglContext, target,
                              reinterpret_cast<EGLClientBuffer>(buffer), NULL);

  return static_cast<HandleType>(reinterpret_cast<uintptr_t>(image));
}

EGLBoolean Renderer::destroyClientImage(HandleType image) {
  return s_egl.eglDestroyImageKHR(m_eglDisplay,
                                  reinterpret_cast<EGLImageKHR>(image));
}

//
// The framebuffer lock should be held when calling this function !
//
bool Renderer::bind_locked() {
  EGLContext prevContext = s_egl.eglGetCurrentContext();
  EGLSurface prevReadSurf = s_egl.eglGetCurrentSurface(EGL_READ);
  EGLSurface prevDrawSurf = s_egl.eglGetCurrentSurface(EGL_DRAW);

  if (!s_egl.eglMakeCurrent(m_eglDisplay, m_pbufSurface, m_pbufSurface,
                            m_pbufContext)) {
    ERROR("eglMakeCurrent failed: 0x%04x", s_egl.eglGetError());
    return false;
  }

  m_prevContext = prevContext;
  m_prevReadSurf = prevReadSurf;
  m_prevDrawSurf = prevDrawSurf;
  return true;
}

bool Renderer::bindWindow_locked(RendererWindow *window) {
  EGLContext prevContext = s_egl.eglGetCurrentContext();
  EGLSurface prevReadSurf = s_egl.eglGetCurrentSurface(EGL_READ);
  EGLSurface prevDrawSurf = s_egl.eglGetCurrentSurface(EGL_DRAW);

  if (!s_egl.eglMakeCurrent(m_eglDisplay, window->surface, window->surface,
                            m_eglContext)) {
    ERROR("eglMakeCurrent failed");
    return false;
  }

  m_prevContext = prevContext;
  m_prevReadSurf = prevReadSurf;
  m_prevDrawSurf = prevDrawSurf;
  return true;
}

bool Renderer::unbind_locked() {
  if (!s_egl.eglMakeCurrent(m_eglDisplay, m_prevDrawSurf, m_prevReadSurf,
                            m_prevContext)) {
    return false;
  }

  m_prevContext = EGL_NO_CONTEXT;
  m_prevReadSurf = EGL_NO_SURFACE;
  m_prevDrawSurf = EGL_NO_SURFACE;
  return true;
}

const GLchar *const Renderer::vshader = {
    "attribute vec3 position;"
    "attribute vec2 texcoord;"
    "uniform mat4 screen_to_gl_coords;"
    "uniform mat4 display_transform;"
    "uniform mat4 transform;"
    "uniform vec2 center;"
    "varying vec2 v_texcoord;"
    "void main() {"
    "   vec4 mid = vec4(center, 0.0, 0.0);"
    "   vec4 transformed = (transform * (vec4(position, 1.0) - mid)) + mid;"
    "   gl_Position = display_transform * screen_to_gl_coords * transformed;"
    "   v_texcoord = texcoord;"
    "}"};

const GLchar *const Renderer::alphaFShader = {
    "precision mediump float;"
    "uniform sampler2D tex;"
    "uniform float alpha;"
    "varying vec2 v_texcoord;"
    "void main() {"
    "   vec4 frag = texture2D(tex, v_texcoord);"
    "   gl_FragColor = alpha*frag;"
    "}"};

const GLchar *const Renderer::defaultFShader =
    {  // This is the fastest fragment shader. Use it when you can.
        "precision mediump float;"
        "uniform sampler2D tex;"
        "varying vec2 v_texcoord;"
        "void main() {"
        "   gl_FragColor = texture2D(tex, v_texcoord);"
        "}"};

void Renderer::setupViewport(RendererWindow *window,
                             const anbox::graphics::Rect &rect) {
  /*
   * Here we provide a 3D perspective projection with a default 30 degrees
   * vertical field of view. This projection matrix is carefully designed
   * such that any vertices at depth z=0 will fit the screen coordinates. So
   * client texels will fit screen pixels perfectly as long as the surface is
   * at depth zero. But if you want to do anything fancy, you can also choose
   * a different depth and it will appear to come out of or go into the
   * screen.
   */
  window->screen_to_gl_coords =
      glm::translate(glm::mat4(1.0f), glm::vec3{-1.0f, 1.0f, 0.0f});

  /*
   * Perspective division is one thing that can't be done in a matrix
   * multiplication. It happens after the matrix multiplications. GL just
   * scales {x,y} by 1/w. So modify the final part of the projection matrix
   * to set w ([3]) to be the incoming z coordinate ([2]).
   */
  window->screen_to_gl_coords[2][3] = -1.0f;

  float const vertical_fov_degrees = 30.0f;
  float const near = (rect.height() / 2.0f) /
                     std::tan((vertical_fov_degrees * M_PI / 180.0f) / 2.0f);
  float const far = -near;

  window->screen_to_gl_coords =
      glm::scale(window->screen_to_gl_coords,
                 glm::vec3{2.0f / rect.width(), -2.0f / rect.height(),
                           2.0f / (near - far)});
  window->screen_to_gl_coords = glm::translate(
      window->screen_to_gl_coords, glm::vec3{-rect.left(), -rect.top(), 0.0f});

  window->viewport = rect;
}

void Renderer::tessellate(std::vector<anbox::graphics::Primitive> &primitives,
                          const anbox::graphics::Rect &buf_size,
                          const Renderable &renderable) {
  auto rect = renderable.screen_position();
  GLfloat left = rect.left();
  GLfloat right = rect.right();
  GLfloat top = rect.top();
  GLfloat bottom = rect.bottom();

  anbox::graphics::Primitive rectangle;
  rectangle.tex_id = 0;
  rectangle.type = GL_TRIANGLE_STRIP;

  GLfloat tex_left =
      static_cast<GLfloat>(renderable.crop().left()) / buf_size.width();
  GLfloat tex_top =
      static_cast<GLfloat>(renderable.crop().top()) / buf_size.height();
  GLfloat tex_right =
      static_cast<GLfloat>(renderable.crop().right()) / buf_size.width();
  GLfloat tex_bottom =
      static_cast<GLfloat>(renderable.crop().bottom()) / buf_size.height();

  auto &vertices = rectangle.vertices;
  vertices[0] = {{left, top, 0.0f}, {tex_left, tex_top}};
  vertices[1] = {{left, bottom, 0.0f}, {tex_left, tex_bottom}};
  vertices[2] = {{right, top, 0.0f}, {tex_right, tex_top}};
  vertices[3] = {{right, bottom, 0.0f}, {tex_right, tex_bottom}};

  primitives.resize(1);
  primitives[0] = rectangle;
}

void Renderer::draw(RendererWindow *window, const Renderable &renderable,
                    const Program &prog) {
  const auto &color_buffer = m_colorbuffers.find(renderable.buffer());
  if (color_buffer == m_colorbuffers.end()) return;

  const auto &cb = color_buffer->second.cb;

  s_gles2.glUseProgram(prog.id);
  s_gles2.glUniform1i(prog.tex_uniform, 0);
  s_gles2.glUniformMatrix4fv(prog.display_transform_uniform, 1, GL_FALSE,
                             glm::value_ptr(window->display_transform));
  s_gles2.glUniformMatrix4fv(prog.screen_to_gl_coords_uniform, 1, GL_FALSE,
                             glm::value_ptr(window->screen_to_gl_coords));

  s_gles2.glActiveTexture(GL_TEXTURE0);

  auto const &rect = renderable.screen_position();
  GLfloat centerx = rect.left() + rect.width() / 2.0f;
  GLfloat centery = rect.top() + rect.height() / 2.0f;
  s_gles2.glUniform2f(prog.center_uniform, centerx, centery);

  s_gles2.glUniformMatrix4fv(prog.transform_uniform, 1, GL_FALSE,
                             glm::value_ptr(renderable.transformation()));

  if (prog.alpha_uniform >= 0)
    s_gles2.glUniform1f(prog.alpha_uniform, renderable.alpha());

  s_gles2.glEnableVertexAttribArray(prog.position_attr);
  s_gles2.glEnableVertexAttribArray(prog.texcoord_attr);

  m_primitives.clear();
  tessellate(m_primitives, {
             static_cast<int32_t>(cb->getWidth()),
             static_cast<int32_t>(cb->getHeight())}, renderable);

  for (auto const &p : m_primitives) {
    cb->bind();

    s_gles2.glVertexAttribPointer(prog.position_attr, 3, GL_FLOAT, GL_FALSE,
                                  sizeof(anbox::graphics::Vertex),
                                  &p.vertices[0].position);
    s_gles2.glVertexAttribPointer(prog.texcoord_attr, 2, GL_FLOAT, GL_FALSE,
                                  sizeof(anbox::graphics::Vertex),
                                  &p.vertices[0].texcoord);

    s_gles2.glEnable(GL_BLEND);
    s_gles2.glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                                GL_ONE_MINUS_SRC_ALPHA);

    s_gles2.glDrawArrays(p.type, 0, p.nvertices);
  }

  s_gles2.glDisableVertexAttribArray(prog.texcoord_attr);
  s_gles2.glDisableVertexAttribArray(prog.position_attr);
}

bool Renderer::draw(EGLNativeWindowType native_window,
                    const anbox::graphics::Rect &window_frame,
                    const RenderableList &renderables) {

  std::unique_lock<std::mutex> l(m_lock);

  auto w = m_nativeWindows.find(native_window);
  if (w == m_nativeWindows.end()) return false;

  if (!bindWindow_locked(w->second))
    return false;

  setupViewport(w->second, window_frame);
  s_gles2.glViewport(0, 0, window_frame.width(), window_frame.height());
  s_gles2.glClearColor(0.0, 0.0, 0.0, 1.0);
  s_gles2.glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  s_gles2.glClear(GL_COLOR_BUFFER_BIT);

  for (const auto &r : renderables)
    draw(w->second, r, r.alpha() < 1.0f ? m_alphaProgram : m_defaultProgram);

  s_egl.eglSwapBuffers(m_eglDisplay, w->second->surface);

  unbind_locked();

  return false;
}
