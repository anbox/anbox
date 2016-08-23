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

#include "GLcommon/GLDispatch.h"
#include "GLcommon/GLLibrary.h"

#include "emugl/common/lazy_instance.h"
#include "emugl/common/shared_library.h"

#include "OpenglCodecCommon/ErrorLog.h"

#ifdef __linux__
#include <GL/glx.h>
#elif defined(WIN32)
#include <windows.h>
#endif

#include <stdio.h>

typedef GlLibrary::GlFunctionPointer GL_FUNC_PTR;

static GL_FUNC_PTR getGLFuncAddress(const char *funcName, GlLibrary* glLib) {
    return glLib->findSymbol(funcName);
}

#define LOAD_GL_FUNC(return_type, func_name, signature, args)  do { \
        if (!func_name) { \
            void* address = (void *)getGLFuncAddress(#func_name, glLib); \
            if (address) { \
                func_name = (__typeof__(func_name))(address); \
            } else { \
                fprintf(stderr, "Could not load func %s\n", #func_name); \
                func_name = (__typeof__(func_name))(dummy_##func_name); \
            } \
        } \
    } while (0);

#define LOAD_GLEXT_FUNC(return_type, func_name, signature, args) do { \
        if (!func_name) { \
            void* address = (void *)getGLFuncAddress(#func_name, glLib); \
            if (address) { \
                func_name = (__typeof__(func_name))(address); \
            } \
        } \
    } while (0);


// Define dummy functions, only for non-extensions.

#define RETURN_void return
#define RETURN_GLboolean return GL_FALSE
#define RETURN_GLint return 0
#define RETURN_GLuint return 0U
#define RETURN_GLenum return 0
#define RETURN_int return 0
#define RETURN_GLconstubyteptr return NULL

#define RETURN_(x)  RETURN_ ## x

#define DEFINE_DUMMY_FUNCTION(return_type, func_name, signature, args) \
static return_type dummy_##func_name signature { \
    RETURN_(return_type); \
}

#define DEFINE_DUMMY_EXTENSION_FUNCTION(return_type, func_name, signature, args) \
  // nothing here

LIST_GLES_FUNCTIONS(DEFINE_DUMMY_FUNCTION, DEFINE_DUMMY_EXTENSION_FUNCTION)

// Initializing static GLDispatch members*/

emugl::Mutex GLDispatch::s_lock;

#define GL_DISPATCH_DEFINE_POINTER(return_type, function_name, signature, args) \
    GL_APICALL return_type (GL_APIENTRY *GLDispatch::function_name) signature = NULL;

LIST_GLES_FUNCTIONS(GL_DISPATCH_DEFINE_POINTER, GL_DISPATCH_DEFINE_POINTER)

// Constructor.
GLDispatch::GLDispatch() : m_isLoaded(false) {}

void GLDispatch::dispatchFuncs(GLESVersion version, GlLibrary* glLib) {
    emugl::Mutex::AutoLock mutex(s_lock);
    if(m_isLoaded)
        return;

    /* Loading OpenGL functions which are needed for implementing BOTH GLES 1.1 & GLES 2.0*/
    LIST_GLES_COMMON_FUNCTIONS(LOAD_GL_FUNC)
    LIST_GLES_EXTENSIONS_FUNCTIONS(LOAD_GLEXT_FUNC)

    /* Loading OpenGL functions which are needed ONLY for implementing GLES 1.1*/
    if(version == GLES_1_1){
        LIST_GLES1_ONLY_FUNCTIONS(LOAD_GL_FUNC)
        LIST_GLES1_EXTENSIONS_FUNCTIONS(LOAD_GLEXT_FUNC)
    } else if (version == GLES_2_0){
        LIST_GLES2_ONLY_FUNCTIONS(LOAD_GL_FUNC)
        LIST_GLES2_EXTENSIONS_FUNCTIONS(LOAD_GLEXT_FUNC)
    }

    /* Load glGetStringi() if it is available, so use LOAD_GLEXT_FUNC */
    LIST_GLES3_ONLY_FUNCTIONS(LOAD_GLEXT_FUNC)

    m_isLoaded = true;
}
