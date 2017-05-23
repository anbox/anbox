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
#ifndef _LIBRENDER_FRAMEBUFFER_H
#define _LIBRENDER_FRAMEBUFFER_H

#include "anbox/graphics/emugl/ColorBuffer.h"
#include "anbox/graphics/emugl/RenderContext.h"
#include "anbox/graphics/emugl/RendererConfig.h"
#include "anbox/graphics/emugl/TextureDraw.h"
#include "anbox/graphics/emugl/WindowSurface.h"
#include "anbox/graphics/emugl/Renderable.h"

#include "anbox/graphics/primitives.h"
#include "anbox/graphics/program_family.h"
#include "anbox/graphics/renderer.h"

#include <EGL/egl.h>

#include <map>
#include <mutex>

#include <stdint.h>

// Type of handles, a.k.a. "object names" in the GL specification.
// These are integers used to uniquely identify a resource of a given type.
typedef uint32_t HandleType;

struct ColorBufferRef {
  ColorBufferPtr cb;
  uint32_t refcount;  // number of client-side references
};
typedef std::map<HandleType, RenderContextPtr> RenderContextMap;
typedef std::map<HandleType, std::pair<WindowSurfacePtr, HandleType>>
    WindowSurfaceMap;
typedef std::map<HandleType, ColorBufferRef> ColorBufferMap;

// A structure used to list the capabilities of the underlying EGL
// implementation that the FrameBuffer instance depends on.
// |has_eglimage_texture_2d| is true iff the EGL_KHR_gl_texture_2D_image
// extension is supported.
// |has_eglimage_renderbuffer| is true iff the EGL_KHR_gl_renderbuffer_image
// extension is supported.
// |eglMajor| and |eglMinor| are the major and minor version numbers of
// the underlying EGL implementation.
struct RendererCaps {
  bool has_eglimage_texture_2d;
  bool has_eglimage_renderbuffer;
  EGLint eglMajor;
  EGLint eglMinor;
};

struct RendererWindow;

// The FrameBuffer class holds the global state of the emulation library on
// top of the underlying EGL/GLES implementation. It should probably be
// named "Display" instead of "FrameBuffer".
//
// There is only one global instance, that can be retrieved with getFB(),
// and which must be previously setup by calling initialize().
//
class Renderer : public anbox::graphics::Renderer {
 public:
  Renderer();
  virtual ~Renderer();

  // Initialize the global instance.
  // |width| and |height| are the dimensions of the emulator GPU display
  // in pixels. |useSubWindow| is true to indicate that the caller
  // will use setupSubWindow() to let EmuGL display the GPU content in its
  // own sub-windows. If false, this means the caller will use
  // setPostCallback() instead to retrieve the content.
  // Returns true on success, false otherwise.
  bool initialize(EGLNativeDisplayType nativeDisplay);

  // Finalize the instance.
  void finalize();

  // Return the capabilities of the underlying display.
  const RendererCaps& getCaps() const { return m_caps; }

  // Return the list of configs available from this display.
  const RendererConfigList* getConfigs() const { return m_configs; }

  // Retrieve the GL strings of the underlying EGL/GLES implementation.
  // On return, |*vendor|, |*renderer| and |*version| will point to strings
  // that are owned by the instance (and must not be freed by the caller).
  void getGLStrings(const char** vendor, const char** renderer,
                    const char** version) const {
    *vendor = m_glVendor;
    *renderer = m_glRenderer;
    *version = m_glVersion;
  }

  RendererWindow* createNativeWindow(EGLNativeWindowType native_window);
  void destroyNativeWindow(RendererWindow* window);
  void destroyNativeWindow(EGLNativeWindowType native_window);

  // Create a new RenderContext instance for this display instance.
  // |p_config| is the index of one of the configs returned by getConfigs().
  // |p_share| is either EGL_NO_CONTEXT or the handle of a shared context.
  // |p_isGL2| is true to create a GLES 2.x context, or false for a GLES 1.x
  // one.
  // Return a new handle value, which will be 0 in case of error.
  HandleType createRenderContext(int p_config, HandleType p_share,
                                 bool p_isGL2 = false);

  // Create a new WindowSurface instance from this display instance.
  // |p_config| is the index of one of the configs returned by getConfigs().
  // |p_width| and |p_height| are the window dimensions in pixels.
  // Return a new handle value, or 0 in case of error.
  HandleType createWindowSurface(int p_config, int p_width, int p_height);

  // Create a new ColorBuffer instance from this display instance.
  // |p_width| and |p_height| are its dimensions in pixels.
  // |p_internalFormat| is the pixel format. See ColorBuffer::create() for
  // list of valid values. Note that ColorBuffer instances are reference-
  // counted. Use openColorBuffer / closeColorBuffer to operate on the
  // internal count.
  HandleType createColorBuffer(int p_width, int p_height,
                               GLenum p_internalFormat);

  // Call this function when a render thread terminates to destroy all
  // the remaining contexts it created. Necessary to avoid leaking host
  // contexts when a guest application crashes, for example.
  void drainRenderContext();

  // Call this function when a render thread terminates to destroy all
  // remaining window surfqce it created. Necessary to avoid leaking
  // host buffers when a guest application crashes, for example.
  void drainWindowSurface();

  // Destroy a given RenderContext instance. |p_context| is its handle
  // value as returned by createRenderContext().
  void DestroyRenderContext(HandleType p_context);

  // Destroy a given WindowSurface instance. |p_surcace| is its handle
  // value as returned by createWindowSurface().
  void DestroyWindowSurface(HandleType p_surface);

  // Increment the reference count associated with a given ColorBuffer
  // instance. |p_colorbuffer| is its handle value as returned by
  // createColorBuffer().
  int openColorBuffer(HandleType p_colorbuffer);

  // Decrement the reference count associated with a given ColorBuffer
  // instance. |p_colorbuffer| is its handle value as returned by
  // createColorBuffer(). Note that if the reference count reaches 0,
  // the instance is destroyed automatically.
  void closeColorBuffer(HandleType p_colorbuffer);

  // Equivalent for eglMakeCurrent() for the current display.
  // |p_context|, |p_drawSurface| and |p_readSurface| are the handle values
  // of the context, the draw surface and the read surface, respectively.
  // Returns true on success, false on failure.
  // Note: if all handle values are 0, this is an unbind operation.
  bool bindContext(HandleType p_context, HandleType p_drawSurface,
                   HandleType p_readSurface);

  // Attach a ColorBuffer to a WindowSurface instance.
  // See the documentation for WindowSurface::setColorBuffer().
  // |p_surface| is the target WindowSurface's handle value.
  // |p_colorbuffer| is the ColorBuffer handle value.
  // Returns true on success, false otherwise.
  bool setWindowSurfaceColorBuffer(HandleType p_surface,
                                   HandleType p_colorbuffer);

  // Copy the content of a WindowSurface's Pbuffer to its attached
  // ColorBuffer. See the documentation for WindowSurface::flushColorBuffer()
  // |p_surface| is the target WindowSurface's handle value.
  // Returns true on success, false on failure.
  bool flushWindowSurfaceColorBuffer(HandleType p_surface);

  // Bind the current context's EGL_TEXTURE_2D texture to a ColorBuffer
  // instance's EGLImage. This is intended to implement
  // glEGLImageTargetTexture2DOES() for all GLES versions.
  // |p_colorbuffer| is the ColorBuffer's handle value.
  // Returns true on success, false on failure.
  bool bindColorBufferToTexture(HandleType p_colorbuffer);

  // Bind the current context's EGL_RENDERBUFFER_OES render buffer to this
  // ColorBuffer's EGLImage. This is intended to implement
  // glEGLImageTargetRenderbufferStorageOES() for all GLES versions.
  // |p_colorbuffer| is the ColorBuffer's handle value.
  // Returns true on success, false on failure.
  bool bindColorBufferToRenderbuffer(HandleType p_colorbuffer);

  // Read the content of a given ColorBuffer into client memory.
  // |p_colorbuffer| is the ColorBuffer's handle value. Similar
  // to glReadPixels(), this can be a slow operation.
  // |x|, |y|, |width| and |height| are the position and dimensions of
  // a rectangle whose pixel values will be transfered to the host.
  // |format| indicates the format of the pixel data, e.g. GL_RGB or GL_RGBA.
  // |type| is the type of pixel data, e.g. GL_UNSIGNED_BYTE.
  // |pixels| is the address of a caller-provided buffer that will be filled
  // with the pixel data.
  void readColorBuffer(HandleType p_colorbuffer, int x, int y, int width,
                       int height, GLenum format, GLenum type, void* pixels);

  // Update the content of a given ColorBuffer from client data.
  // |p_colorbuffer| is the ColorBuffer's handle value. Similar
  // to glReadPixels(), this can be a slow operation.
  // |x|, |y|, |width| and |height| are the position and dimensions of
  // a rectangle whose pixel values will be transfered to the GPU
  // |format| indicates the format of the pixel data, e.g. GL_RGB or GL_RGBA.
  // |type| is the type of pixel data, e.g. GL_UNSIGNED_BYTE.
  // |pixels| is the address of a buffer containing the new pixel data.
  // Returns true on success, false otherwise.
  bool updateColorBuffer(HandleType p_colorbuffer, int x, int y, int width,
                         int height, GLenum format, GLenum type, void* pixels);

  bool draw(EGLNativeWindowType native_window,
            const anbox::graphics::Rect& window_frame,
            const RenderableList& renderables) override;

  // Return the host EGLDisplay used by this instance.
  EGLDisplay getDisplay() const { return m_eglDisplay; }

  // Return a TextureDraw instance that can be used with this surfaces
  // and windows created by this instance.
  TextureDraw* getTextureDraw() const { return m_textureDraw; }

  HandleType createClientImage(HandleType context, EGLenum target,
                               GLuint buffer);
  EGLBoolean destroyClientImage(HandleType image);

  // Used internally.
  bool bind_locked();
  bool unbind_locked();

 private:
  HandleType genHandle();

  bool bindWindow_locked(RendererWindow* window);

  void setupViewport(RendererWindow* window, const anbox::graphics::Rect& rect);
  struct Program;
  void draw(RendererWindow* window, const Renderable& renderable,
            const Program& prog);
  void tessellate(std::vector<anbox::graphics::Primitive>& primitives,
                  const anbox::graphics::Rect& buf_size,
                  const Renderable& renderable);

 private:
  static Renderer* s_renderer;
  static HandleType s_nextHandle;
  std::mutex m_lock;
  RendererConfigList* m_configs;
  RendererCaps m_caps;
  EGLDisplay m_eglDisplay;
  RenderContextMap m_contexts;
  WindowSurfaceMap m_windows;
  ColorBufferMap m_colorbuffers;
  ColorBuffer::Helper* m_colorBufferHelper;

  EGLContext m_eglContext;
  EGLSurface m_pbufSurface;
  EGLContext m_pbufContext;

  EGLContext m_prevContext;
  EGLSurface m_prevReadSurf;
  EGLSurface m_prevDrawSurf;
  TextureDraw* m_textureDraw;
  EGLConfig m_eglConfig;
  HandleType m_lastPostedColorBuffer;

  int m_statsNumFrames;
  long long m_statsStartTime;
  bool m_fpsStats;

  const char* m_glVendor;
  const char* m_glRenderer;
  const char* m_glVersion;

  std::map<EGLNativeWindowType, RendererWindow*> m_nativeWindows;

  anbox::graphics::ProgramFamily m_family;
  struct Program {
    GLuint id = 0;
    GLint tex_uniform = -1;
    GLint position_attr = -1;
    GLint texcoord_attr = -1;
    GLint center_uniform = -1;
    GLint display_transform_uniform = -1;
    GLint transform_uniform = -1;
    GLint screen_to_gl_coords_uniform = -1;
    GLint alpha_uniform = -1;
    mutable long long last_used_frameno = 0;

    Program(GLuint program_id);
    Program() {}
  };
  Program m_defaultProgram, m_alphaProgram;

  std::vector<anbox::graphics::Primitive> m_primitives;

  static const GLchar* const vshader;
  static const GLchar* const defaultFShader;
  static const GLchar* const alphaFShader;
};
#endif
