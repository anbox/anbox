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

#include "anbox/graphics/emugl/WindowSurface.h"
#include "anbox/graphics/emugl/RendererConfig.h"
#include "anbox/logger.h"

#include "external/android-emugl/host/include/OpenGLESDispatch/EGLDispatch.h"

#include <GLES/glext.h>

#include <stdio.h>
#include <string.h>


WindowSurface::WindowSurface(EGLDisplay display, EGLConfig config)
    : mSurface(NULL),
      mAttachedColorBuffer(NULL),
      mReadContext(NULL),
      mDrawContext(NULL),
      mWidth(0),
      mHeight(0),
      mConfig(config),
      mDisplay(display) {}

WindowSurface::~WindowSurface() {
  if (mSurface) {
    s_egl.eglDestroySurface(mDisplay, mSurface);
  }
}

WindowSurface *WindowSurface::create(EGLDisplay display, EGLConfig config,
                                     int p_width, int p_height) {
  // allocate space for the WindowSurface object
  WindowSurface *win = new WindowSurface(display, config);
  if (!win) {
    return NULL;
  }

  // Create a pbuffer to be used as the egl surface
  // for that window.
  if (!win->resize(p_width, p_height)) {
    delete win;
    return NULL;
  }

  return win;
}

void WindowSurface::setColorBuffer(ColorBufferPtr p_colorBuffer) {
  mAttachedColorBuffer = p_colorBuffer;

  // resize the window if the attached color buffer is of different
  // size.
  unsigned int cbWidth = mAttachedColorBuffer->getWidth();
  unsigned int cbHeight = mAttachedColorBuffer->getHeight();

  if (cbWidth != mWidth || cbHeight != mHeight) {
    resize(cbWidth, cbHeight);
  }
}

void WindowSurface::bind(RenderContextPtr p_ctx, BindType p_bindType) {
  if (p_bindType == BIND_READ) {
    mReadContext = p_ctx;
  } else if (p_bindType == BIND_DRAW) {
    mDrawContext = p_ctx;
  } else if (p_bindType == BIND_READDRAW) {
    mReadContext = p_ctx;
    mDrawContext = p_ctx;
  }
}

bool WindowSurface::flushColorBuffer() {
  if (!mAttachedColorBuffer) {
    return true;
  }
  if (!mWidth || !mHeight) {
    return false;
  }

  if (mAttachedColorBuffer->getWidth() != mWidth ||
      mAttachedColorBuffer->getHeight() != mHeight) {
    // XXX: should never happen - how this needs to be handled?
    ERROR("Dimensions do not match");
    return false;
  }

  if (!mDrawContext) {
    ERROR("Draw context is NULL");
    return false;
  }

  // Make the surface current
  EGLContext prevContext = s_egl.eglGetCurrentContext();
  EGLSurface prevReadSurf = s_egl.eglGetCurrentSurface(EGL_READ);
  EGLSurface prevDrawSurf = s_egl.eglGetCurrentSurface(EGL_DRAW);

  if (!s_egl.eglMakeCurrent(mDisplay, mSurface, mSurface,
                            mDrawContext->getEGLContext())) {
    ERROR("Failed to make draw context current");
    return false;
  }

  mAttachedColorBuffer->blitFromCurrentReadBuffer();

  // restore current context/surface
  s_egl.eglMakeCurrent(mDisplay, prevDrawSurf, prevReadSurf, prevContext);

  return true;
}

bool WindowSurface::resize(unsigned int p_width, unsigned int p_height) {
  if (mSurface && mWidth == p_width && mHeight == p_height) {
    // no need to resize
    return true;
  }

  EGLContext prevContext = s_egl.eglGetCurrentContext();
  EGLSurface prevReadSurf = s_egl.eglGetCurrentSurface(EGL_READ);
  EGLSurface prevDrawSurf = s_egl.eglGetCurrentSurface(EGL_DRAW);
  EGLSurface prevPbuf = mSurface;
  bool needRebindContext =
      mSurface && (prevReadSurf == mSurface || prevDrawSurf == mSurface);

  if (needRebindContext) {
    s_egl.eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
                         EGL_NO_CONTEXT);
  }

  if (mSurface) {
    s_egl.eglDestroySurface(mDisplay, mSurface);
    mSurface = NULL;
  }

  const EGLint pbufAttribs[5] = {
      EGL_WIDTH, static_cast<EGLint>(p_width),
      EGL_HEIGHT, static_cast<EGLint>(p_height),
      EGL_NONE,
  };

  mSurface = s_egl.eglCreatePbufferSurface(mDisplay, mConfig, pbufAttribs);
  if (mSurface == EGL_NO_SURFACE) {
    ERROR("Failed to create/resize pbuffer");
    return false;
  }

  mWidth = p_width;
  mHeight = p_height;

  if (needRebindContext) {
    s_egl.eglMakeCurrent(
        mDisplay, (prevDrawSurf == prevPbuf) ? mSurface : prevDrawSurf,
        (prevReadSurf == prevPbuf) ? mSurface : prevReadSurf, prevContext);
  }

  return true;
}
