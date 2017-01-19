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

#ifndef _LIBRENDER_FB_CONFIG_H
#define _LIBRENDER_FB_CONFIG_H

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <stddef.h>

// A class used to model a guest EGL config.
// This really wraps a host EGLConfig handle, and provides a few cached
// attributes that can be retrieved through direct accessors, like
// getDepthSize().
//
// Each FbConfig is identified by a unique id which is its index in
// an FbConfigList instance (as declared below). It is not related to
// the host EGLConfig value or its EGL_CONFIG_ID.
//
// One doesn't create an FbConfig instance. Instead, create and initialize
// an FbConfigList from the host EGLDisplay, and use its size() and get()
// methods to access it.
class RendererConfig {
 public:
  // Destructor
  ~RendererConfig();

  // Retrieve host EGLConfig.
  EGLConfig getEglConfig() const { return mEglConfig; }

  // Get depth size in bits.
  GLuint getDepthSize() const { return getAttribValue(0); }

  // Get stencil size in bits.
  GLuint getStencilSize() const { return getAttribValue(1); }

  // Get renderable type mask.
  GLuint getRenderableType() const { return getAttribValue(2); }

  // Get surface type mask.
  GLuint getSurfaceType() const { return getAttribValue(3); }

  // Get the EGL_CONFIG_ID value. This is the same as the one of the
  // underlying host EGLConfig handle.
  GLint getConfigId() const { return static_cast<GLint>(getAttribValue(4)); }

 private:
  RendererConfig();
  RendererConfig(RendererConfig& other);

  explicit RendererConfig(EGLConfig hostConfig, EGLDisplay hostDisplay);

  friend class RendererConfigList;

  GLuint getAttribValue(int n) const {
    return mAttribValues ? mAttribValues[n] : 0U;
  }

  EGLConfig mEglConfig;
  GLint* mAttribValues;
};

// A class to model the list of FbConfig for a given EGLDisplay, this is
// built from the list of host EGLConfig handles, filtered to only accept
// configs that are useful by the rendering library (e.g. they must support
// PBuffers and RGB pixel values).
//
// Usage is the following:
//
// 1) Create new instance by passing host EGLDisplay value.
//
// 2) Call empty() to check that the list is not empty, which would indicate
//    an error during creation.
//
// 3) FbConfig instances are identified by numbers in 0..(N-1) range, where
//    N is the result of the size() method.
//
// 4) Convert an FbConfig id into an FbConfig instance with get().
//
// 5) Use getPackInfo() and packConfigs() to retrieve information about
//    available configs to the guest.
class RendererConfigList {
 public:
  // Create a new list of FbConfig instance, by querying all compatible
  // host configs from |display|. A compatible config is one that supports
  // Pbuffers and RGB pixel values.
  //
  // After construction, call empty() to check if there are items.
  // An empty list means there was an error during construction.
  explicit RendererConfigList(EGLDisplay display);

  // Destructor.
  ~RendererConfigList();

  // Return true iff the list is empty. true means there was an error
  // during construction.
  bool empty() const { return mCount == 0; }

  // Return the number of FbConfig instances in the list.
  // Each instance is identified by a number from 0 to N-1,
  // where N is the result of this function.
  size_t size() const { return static_cast<size_t>(mCount); }

  // Retrieve the FbConfig instance associated with |guestId|,
  // which must be an integer between 0 and |size() - 1|. Returns
  // NULL in case of failure.
  const RendererConfig* get(int guestId) const {
    if (guestId >= 0 && guestId < mCount) {
      return mConfigs[guestId];
    } else {
      return NULL;
    }
  }

  // Use |attribs| a list of EGL attribute name/values terminated by
  // EGL_NONE, to select a set of matching FbConfig instances.
  //
  // On success, returns the number of matching instances.
  // If |configs| is not NULL, it will be populated with the guest IDs
  // of the matched FbConfig instances.
  //
  // |configsSize| is the number of entries in the |configs| array. The
  // function will never write more than |configsSize| entries into
  // |configsSize|.
  EGLint chooseConfig(const EGLint* attribs, EGLint* configs,
                      EGLint configsSize) const;

  // Retrieve information that can be sent to the guest before packed
  // config list information. If |numConfigs| is NULL, then |*numConfigs|
  // will be set on return to the number of config instances.
  // If |numAttribs| is not NULL, then |*numAttribs| will be set on return
  // to the number of attribute values cached by each FbConfig instance.
  void getPackInfo(EGLint* mumConfigs, EGLint* numAttribs) const;

  // Write the full list information into an array of EGLuint items.
  // |buffer| is the output buffer that will receive the data.
  // |bufferByteSize| is teh buffer size in bytes.
  // On success, this returns
  EGLint packConfigs(GLuint bufferByteSize, GLuint* buffer) const;

 private:
  RendererConfigList();
  RendererConfigList(const RendererConfigList& other);

  int mCount;
  RendererConfig** mConfigs;
  EGLDisplay mDisplay;
};

#endif  // _LIBRENDER_FB_CONFIG_H
