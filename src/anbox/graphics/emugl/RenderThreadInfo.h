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

#ifndef _LIB_OPENGL_RENDER_THREAD_INFO_H
#define _LIB_OPENGL_RENDER_THREAD_INFO_H

#include "anbox/graphics/emugl/RenderContext.h"
#include "anbox/graphics/emugl/WindowSurface.h"

#include "external/android-emugl/host/libs/GLESv1_dec/GLESv1Decoder.h"
#include "external/android-emugl/host/libs/GLESv2_dec/GLESv2Decoder.h"

// Generated with emugl at build time
#include "renderControl_dec.h"

#include <set>

typedef uint32_t HandleType;
typedef std::set<HandleType> ThreadContextSet;
typedef std::set<HandleType> WindowSurfaceSet;

// A class used to model the state of each RenderThread related
struct RenderThreadInfo {
  // Create new instance. Only call this once per thread.
  // Future callls to get() will return this instance until
  // it is destroyed.
  RenderThreadInfo();

  // Destructor.
  ~RenderThreadInfo();

  // Return the current thread's instance, if any, or NULL.
  static RenderThreadInfo* get();

  // Current EGL context, draw surface and read surface.
  RenderContextPtr currContext;
  WindowSurfacePtr currDrawSurf;
  WindowSurfacePtr currReadSurf;

  // Decoder states.
  GLESv1Decoder m_glDec;
  GLESv2Decoder m_gl2Dec;
  renderControl_decoder_context_t m_rcDec;

  // all the contexts that are created by this render thread
  ThreadContextSet m_contextSet;
  // all the window surfaces that are created by this render thread
  WindowSurfaceSet m_windowSet;
};

#endif
