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
#ifndef _LIBRENDER_COLORBUFFER_H
#define _LIBRENDER_COLORBUFFER_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>

#include <memory>

class TextureDraw;
class TextureResize;

// A class used to model a guest color buffer, and used to implement several
// related things:
//
//  - Every gralloc native buffer with HW read or write requirements will
//    allocate a host ColorBuffer instance. When gralloc_lock() is called,
//    the guest will use ColorBuffer::readPixels() to read the current content
//    of the buffer. When gralloc_unlock() is later called, it will call
//    ColorBuffer::subUpdate() to send the updated pixels.
//
//  - Every guest window EGLSurface is implemented by a host PBuffer
//    (see WindowSurface.h) that can have a ColorBuffer instance attached to
//    it (through WindowSurface::attachColorBuffer()). When such an attachment
//    exists, WindowSurface::flushColorBuffer() will copy the PBuffer's
//    pixel data into the ColorBuffer. The latter can then be displayed
//    in the client's UI sub-window with ColorBuffer::post().
//
//  - Guest EGLImages are implemented as native gralloc buffers too.
//    The guest glEGLImageTargetTexture2DOES() implementations will end up
//    calling ColorBuffer::bindToTexture() to bind the current context's
//    GL_TEXTURE_2D to the buffer. Similarly, the guest versions of
//    glEGLImageTargetRenderbufferStorageOES() will end up calling
//    ColorBuffer::bindToRenderbuffer().
//
// This forces the implementation to use a host EGLImage to implement each
// ColorBuffer.
//
// As an additional twist.

class ColorBuffer {
 public:
  // Helper interface class used during ColorBuffer operations. This is
  // introduced to remove coupling from the FrameBuffer class implementation.
  class Helper {
   public:
    Helper() {}
    virtual ~Helper() {}
    virtual bool setupContext() = 0;
    virtual void teardownContext() = 0;
    virtual TextureDraw* getTextureDraw() const = 0;
  };

  // Create a new ColorBuffer instance.
  // |p_display| is the host EGLDisplay handle.
  // |p_width| and |p_height| are the buffer's dimensions in pixels.
  // |p_internalFormat| is the internal pixel format to use, valid values
  // are: GL_RGB, GL_RGB565, GL_RGBA, GL_RGB5_A1_OES and GL_RGBA4_OES.
  // Implementation is free to use something else though.
  // |has_eglimage_texture_2d| should be true iff the display supports
  // the EGL_KHR_gl_texture_2D_image extension.
  // Returns NULL on failure.
  static ColorBuffer* create(EGLDisplay p_display, int p_width, int p_height,
                             GLenum p_internalFormat,
                             bool has_eglimage_texture_2d, Helper* helper);

  // Destructor.
  ~ColorBuffer();

  // Return ColorBuffer width and height in pixels
  GLuint getWidth() const { return m_width; }
  GLuint getHeight() const { return m_height; }

  // Read the ColorBuffer instance's pixel values into host memory.
  void readPixels(int x, int y, int width, int height, GLenum p_format,
                  GLenum p_type, void* pixels);

  // Update the ColorBuffer instance's pixel values from host memory.
  void subUpdate(int x, int y, int width, int height, GLenum p_format,
                 GLenum p_type, void* pixels);

  // Bind the current context's EGL_TEXTURE_2D texture to this ColorBuffer's
  // EGLImage. This is intended to implement glEGLImageTargetTexture2DOES()
  // for all GLES versions.
  bool bindToTexture();

  // Bind the current context's EGL_RENDERBUFFER_OES render buffer to this
  // ColorBuffer's EGLImage. This is intended to implement
  // glEGLImageTargetRenderbufferStorageOES() for all GLES versions.
  bool bindToRenderbuffer();

  // Copy the content of the current context's read surface to this
  // ColorBuffer. This is used from WindowSurface::flushColorBuffer().
  // Return true on success, false on failure (e.g. no current context).
  bool blitFromCurrentReadBuffer();

  // Read the content of the whole ColorBuffer as 32-bit RGBA pixels.
  // |img| must be a buffer large enough (i.e. width * height * 4).
  void readback(unsigned char* img);

  void bind();

 private:
  ColorBuffer();  // no default constructor.

  explicit ColorBuffer(EGLDisplay display, Helper* helper);

 private:
  GLuint m_tex;
  GLuint m_blitTex;
  EGLImageKHR m_eglImage;
  EGLImageKHR m_blitEGLImage;
  GLuint m_width;
  GLuint m_height;
  GLuint m_fbo;
  GLenum m_internalFormat;
  EGLDisplay m_display;
  Helper* m_helper;
  TextureResize* m_resizer;
};

typedef std::shared_ptr<ColorBuffer> ColorBufferPtr;

#endif
