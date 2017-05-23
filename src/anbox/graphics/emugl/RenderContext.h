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

#ifndef _LIBRENDER_RENDER_CONTEXT_H
#define _LIBRENDER_RENDER_CONTEXT_H

#include "external/android-emugl/shared/OpenglCodecCommon/GLDecoderContextData.h"

#include <EGL/egl.h>

#include <memory>

// A class used to model a guest EGLContext. This simply wraps a host
// EGLContext, associated with an GLDecoderContextData instance that is
// used to store copies of guest-side arrays.
class RenderContext {
 public:
  // Create a new RenderContext instance.
  // |display| is the host EGLDisplay handle.
  // |config| is the host EGLConfig to use.
  // |sharedContext| is either EGL_NO_CONTEXT of a host EGLContext handle.
  // |isGl2| is true iff the new context will be used with GLESv2, or
  // GLESv1 otherwise.
  static RenderContext* create(EGLDisplay display, EGLConfig config,
                               EGLContext sharedContext, bool isGL2 = false);

  // Destructor.
  ~RenderContext();

  // Retrieve host EGLContext value.
  EGLContext getEGLContext() const { return mContext; }

  // Return true iff this is a GLESv2 context.
  bool isGL2() const { return mIsGl2; }

  // Retrieve GLDecoderContextData instance reference for this
  // RenderContext instance.
  GLDecoderContextData& decoderContextData() { return mContextData; }

 private:
  RenderContext();

  RenderContext(EGLDisplay display, EGLContext context, bool isGl2);

 private:
  EGLDisplay mDisplay;
  EGLContext mContext;
  bool mIsGl2;
  GLDecoderContextData mContextData;
};

typedef std::shared_ptr<RenderContext> RenderContextPtr;

#endif  // _LIBRENDER_RENDER_CONTEXT_H
