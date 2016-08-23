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
#ifndef EGL_VALIDATE_H
#define EGL_VALIDATE_H

#include <EGL/egl.h>

class EglValidate {
public:
    // Return true iff |attrib| is a valid EGLConfig attribute name.
    static bool confAttrib(EGLint attrib);

    // Return true iff |attrib| is NULL, or |attrib[0]| is EGL_NONE, which
    // correspond to empty config attribute lists.
    static bool noAttribs(const EGLint* attrib);

    // Used to check that the config attributes used to create a new pbuffer
    // are correct. |width| and |height| are the PBuffer's dimensions in
    // pixels. |texFormatIsNoText| is true iff the PBuffer's texture format
    // if EGL_NO_TEXTURE. |texTargetIsNoTex| is true iff the PBuffer's target
    // is EGL_NO_TEXTURE.
    static bool pbufferAttribs(EGLint width,
                               EGLint height,
                               bool texFormatIsNoTex,
                               bool texTargetIsNoTex);

    // Return true if |ctx| is EGL_NO_CONTEXT, and both |draw| and |read|
    // are EGL_NO_SURFACE. This corresponds to an eglMakeCurrent operation
    // used to release the current context and draw/read surfaces at the
    // same time.
    static bool releaseContext(EGLContext ctx,
                               EGLSurface draw,
                               EGLSurface read);

    // Return true if the values of |ctx|, |draw| and |read| do _not_ match
    // the following rules:
    //
    // - If |ctx| is EGL_NO_CONTEXT, then |draw| and |read| must be
    //   EGL_NO_SURFACE.
    //
    // - If |ctx| is not EGL_NO_CONTEXT, then |draw| and |read| must not
    //   be EGL_NO_SURFACE.
    //
    static bool badContextMatch(EGLContext ctx,
                                EGLSurface draw,
                                EGLSurface read);

    // Returns true iff |target| is either EGL_DRAW or EGL_READ.
    static bool surfaceTarget(EGLint target);

    // Returns true iff |engine| is EGL_CORE_NATIVE_RENDERER
    static bool engine(EGLint engine);

    // Returns true iff |name| is the attribute name of a string, e.g.
    // EGL_VENDOR, EGL_VERSION or EGL_EXTENSIONS.
    static bool stringName(EGLint name);

    // Returns true iff |api| is the value of a supported API, e.g.
    // EGL_OPENGL_ES_API.
    static bool supportedApi(EGLenum api);
};
#endif
