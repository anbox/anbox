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

#include "anbox/graphics/emugl/RenderContext.h"

#include "OpenGLESDispatch/EGLDispatch.h"

RenderContext* RenderContext::create(EGLDisplay display, EGLConfig config,
                                     EGLContext sharedContext, bool isGl2) {
  const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, isGl2 ? 2 : 1,
                                   EGL_NONE};
  EGLContext context =
      s_egl.eglCreateContext(display, config, sharedContext, contextAttribs);
  if (context == EGL_NO_CONTEXT) {
    return NULL;
  }

  return new RenderContext(display, context, isGl2);
}

RenderContext::RenderContext(EGLDisplay display, EGLContext context, bool isGl2)
    : mDisplay(display), mContext(context), mIsGl2(isGl2), mContextData() {}

RenderContext::~RenderContext() {
  if (mContext != EGL_NO_CONTEXT) {
    s_egl.eglDestroyContext(mDisplay, mContext);
  }
}
