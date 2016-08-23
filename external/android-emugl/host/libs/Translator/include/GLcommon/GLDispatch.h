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
#ifndef GL_DISPATCHH
#define GL_DISPATCHH

#include <GLES/gl.h>
#include <GLES2/gl2.h>

#include "OpenGLESDispatch/gldefs.h"
#include "OpenGLESDispatch/gles_functions.h"
#include "GLutils.h"

#include "emugl/common/mutex.h"

#define GLAPIENTRY GL_APIENTRY
typedef void(*FUNCPTR)();

class GlLibrary;

#define GLES_DECLARE_METHOD(return_type, function_name, signature, args) \
    static GL_APICALL return_type (GL_APIENTRY *function_name) signature;

class GLDispatch {
public:
    // Constructor.
    GLDispatch();

    void dispatchFuncs(GLESVersion version, GlLibrary* glLib);

    LIST_GLES_FUNCTIONS(GLES_DECLARE_METHOD, GLES_DECLARE_METHOD)

private:
    bool                  m_isLoaded;
    static emugl::Mutex   s_lock;
};

#endif  // GL_DISPATCH_H
