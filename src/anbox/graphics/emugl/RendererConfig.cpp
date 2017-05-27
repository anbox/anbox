// Copyright (C) 2015 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "anbox/graphics/emugl/RendererConfig.h"
#include "anbox/logger.h"

#include "external/android-emugl/host/include/OpenGLESDispatch/EGLDispatch.h"

#include <stdio.h>
#include <string.h>

namespace {
const GLuint kConfigAttributes[] = {
    EGL_DEPTH_SIZE,       // must be first - see getDepthSize()
    EGL_STENCIL_SIZE,     // must be second - see getStencilSize()
    EGL_RENDERABLE_TYPE,  // must be third - see getRenderableType()
    EGL_SURFACE_TYPE,     // must be fourth - see getSurfaceType()
    EGL_CONFIG_ID,        // must be fifth  - see chooseConfig()
    EGL_BUFFER_SIZE, EGL_ALPHA_SIZE, EGL_BLUE_SIZE, EGL_GREEN_SIZE,
    EGL_RED_SIZE, EGL_CONFIG_CAVEAT, EGL_LEVEL, EGL_MAX_PBUFFER_HEIGHT,
    EGL_MAX_PBUFFER_PIXELS, EGL_MAX_PBUFFER_WIDTH, EGL_NATIVE_RENDERABLE,
    EGL_NATIVE_VISUAL_ID, EGL_NATIVE_VISUAL_TYPE, EGL_SAMPLES,
    EGL_SAMPLE_BUFFERS, EGL_TRANSPARENT_TYPE, EGL_TRANSPARENT_BLUE_VALUE,
    EGL_TRANSPARENT_GREEN_VALUE, EGL_TRANSPARENT_RED_VALUE,
    EGL_BIND_TO_TEXTURE_RGB, EGL_BIND_TO_TEXTURE_RGBA, EGL_MIN_SWAP_INTERVAL,
    EGL_MAX_SWAP_INTERVAL, EGL_LUMINANCE_SIZE, EGL_ALPHA_MASK_SIZE,
    EGL_COLOR_BUFFER_TYPE,
    // EGL_MATCH_NATIVE_PIXMAP,
    EGL_CONFORMANT};

const size_t kConfigAttributesLen =
    sizeof(kConfigAttributes) / sizeof(kConfigAttributes[0]);

bool isCompatibleHostConfig(EGLConfig config, EGLDisplay display) {
  // Filter out configs which do not support pbuffers, since they
  // are used to implement window surfaces.
  EGLint surfaceType;
  s_egl.eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &surfaceType);
  if (!(surfaceType & EGL_PBUFFER_BIT)) {
    return false;
  }

  // Filter out configs that do not support RGB pixel values.
  EGLint redSize = 0, greenSize = 0, blueSize = 0;
  s_egl.eglGetConfigAttrib(display, config, EGL_RED_SIZE, &redSize);
  s_egl.eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &greenSize);
  s_egl.eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blueSize);

  if (!redSize || !greenSize || !blueSize) {
    return false;
  }

  return true;
}
}  // namespace

RendererConfig::~RendererConfig() { delete[] mAttribValues; }

RendererConfig::RendererConfig(EGLConfig hostConfig, EGLDisplay hostDisplay)
    : mEglConfig(hostConfig), mAttribValues(NULL) {
  mAttribValues = new GLint[kConfigAttributesLen];
  for (size_t i = 0; i < kConfigAttributesLen; ++i) {
    mAttribValues[i] = 0;
    s_egl.eglGetConfigAttrib(hostDisplay, hostConfig, kConfigAttributes[i],
                             &mAttribValues[i]);

    // This implementation supports guest window surfaces by wrapping
    // them around host Pbuffers, so always report it to the guest.
    if (kConfigAttributes[i] == EGL_SURFACE_TYPE) {
      mAttribValues[i] |= EGL_WINDOW_BIT;
    }
  }
}

RendererConfigList::RendererConfigList(EGLDisplay display)
    : mCount(0), mConfigs(NULL), mDisplay(display) {
  if (display == EGL_NO_DISPLAY) {
    ERROR("Invalid display value %p (EGL_NO_DISPLAY)", reinterpret_cast<void*>(display));
    return;
  }

  EGLint numHostConfigs = 0;
  if (!s_egl.eglGetConfigs(display, NULL, 0, &numHostConfigs)) {
    ERROR("Could not get number of host EGL config");
    return;
  }
  EGLConfig* hostConfigs = new EGLConfig[numHostConfigs];
  s_egl.eglGetConfigs(display, hostConfigs, numHostConfigs, &numHostConfigs);

  mConfigs = new RendererConfig*[numHostConfigs];
  for (EGLint i = 0; i < numHostConfigs; ++i) {
    // Filter out configs that are not compatible with our implementation.
    if (!isCompatibleHostConfig(hostConfigs[i], display)) {
      continue;
    }
    mConfigs[mCount] = new RendererConfig(hostConfigs[i], display);
    mCount++;
  }

  delete[] hostConfigs;
}

RendererConfigList::~RendererConfigList() {
  for (int n = 0; n < mCount; ++n) {
    delete mConfigs[n];
  }
  delete[] mConfigs;
}

int RendererConfigList::chooseConfig(const EGLint* attribs, EGLint* configs,
                                     EGLint configsSize) const {
  EGLint numHostConfigs = 0;
  if (!s_egl.eglGetConfigs(mDisplay, NULL, 0, &numHostConfigs)) {
    ERROR("Could not get number of host EGL configs");
    return 0;
  }

  EGLConfig* matchedConfigs = new EGLConfig[numHostConfigs];

  // If EGL_SURFACE_TYPE appears in |attribs|, the value passed to
  // eglChooseConfig should be forced to EGL_PBUFFER_BIT because that's
  // what it used by the current implementation, exclusively. This forces
  // the rewrite of |attribs| into a new array.
  bool hasSurfaceType = false;
  bool mustReplaceSurfaceType = false;
  int numAttribs = 0;
  while (attribs[numAttribs] != EGL_NONE) {
    if (attribs[numAttribs] == EGL_SURFACE_TYPE) {
      hasSurfaceType = true;
      if (attribs[numAttribs + 1] != EGL_PBUFFER_BIT) {
        mustReplaceSurfaceType = true;
      }
    }
    numAttribs += 2;
  }

  EGLint* newAttribs = NULL;

  if (mustReplaceSurfaceType) {
    // There is at least on EGL_SURFACE_TYPE in |attribs|. Copy the
    // array and replace all values with EGL_PBUFFER_BIT
    newAttribs = new GLint[numAttribs + 1];
    memcpy(newAttribs, attribs, numAttribs * sizeof(GLint));
    newAttribs[numAttribs] = EGL_NONE;
    for (int n = 0; n < numAttribs; n += 2) {
      if (newAttribs[n] == EGL_SURFACE_TYPE) {
        newAttribs[n + 1] = EGL_PBUFFER_BIT;
      }
    }
  } else if (!hasSurfaceType) {
    // There is no EGL_SURFACE_TYPE in |attribs|, then add one entry
    // with the value EGL_PBUFFER_BIT.
    newAttribs = new GLint[numAttribs + 3];
    memcpy(newAttribs, attribs, numAttribs * sizeof(GLint));
    newAttribs[numAttribs] = EGL_SURFACE_TYPE;
    newAttribs[numAttribs + 1] = EGL_PBUFFER_BIT;
    newAttribs[numAttribs + 2] = EGL_NONE;
  }

  if (!s_egl.eglChooseConfig(mDisplay, newAttribs ? newAttribs : attribs,
                             matchedConfigs, numHostConfigs, &numHostConfigs)) {
    numHostConfigs = 0;
  }

  delete[] newAttribs;

  int result = 0;
  for (int n = 0; n < numHostConfigs; ++n) {
    // Don't count or write more than |configsSize| items if |configs|
    // is not NULL.
    if (configs && configsSize > 0 && result >= configsSize) {
      break;
    }
    // Skip incompatible host configs.
    if (!isCompatibleHostConfig(matchedConfigs[n], mDisplay)) {
      continue;
    }
    // Find the FbConfig with the same EGL_CONFIG_ID
    EGLint hostConfigId;
    s_egl.eglGetConfigAttrib(mDisplay, matchedConfigs[n], EGL_CONFIG_ID,
                             &hostConfigId);
    for (int k = 0; k < mCount; ++k) {
      int guestConfigId = mConfigs[k]->getConfigId();
      if (guestConfigId == hostConfigId) {
        // There is a match. Write it to |configs| if it is not NULL.
        if (configs && result < configsSize) {
          configs[result] = static_cast<uint32_t>(k);
        }
        result++;
        break;
      }
    }
  }

  delete[] matchedConfigs;

  return result;
}

void RendererConfigList::getPackInfo(EGLint* numConfigs,
                                     EGLint* numAttributes) const {
  if (numConfigs) {
    *numConfigs = mCount;
  }
  if (numAttributes) {
    *numAttributes = static_cast<EGLint>(kConfigAttributesLen);
  }
}

EGLint RendererConfigList::packConfigs(GLuint bufferByteSize,
                                       GLuint* buffer) const {
  GLuint numAttribs = static_cast<GLuint>(kConfigAttributesLen);
  GLuint kGLuintSize = static_cast<GLuint>(sizeof(GLuint));
  GLuint neededByteSize = (mCount + 1) * numAttribs * kGLuintSize;
  if (!buffer || bufferByteSize < neededByteSize) {
    return -neededByteSize;
  }
  // Write to the buffer the config attribute ids, followed for each one
  // of the configs, their values.
  memcpy(buffer, kConfigAttributes, kConfigAttributesLen * kGLuintSize);

  for (int i = 0; i < mCount; ++i) {
    memcpy(buffer + (i + 1) * kConfigAttributesLen, mConfigs[i]->mAttribValues,
           kConfigAttributesLen * kGLuintSize);
  }
  return mCount;
}
