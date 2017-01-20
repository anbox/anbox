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
#include "OpenGLESDispatch/GLESv2Dispatch.h"
#include "OpenGLESDispatch/EGLDispatch.h"

#include <stdio.h>
#include <stdlib.h>

#include "emugl/common/shared_library.h"

extern EGLDispatch s_egl;

static emugl::SharedLibrary *s_gles2_lib = NULL;

namespace {
constexpr const char *glesv2_lib_env_var{"ANDROID_GLESv2_LIB"};
}

// An unimplemented function which prints out an error message.
// To make it consistent with the guest, all GLES2 functions not supported by
// the driver should be redirected to this function.

static void gles2_unimplemented() {
    fprintf(stderr, "Called unimplemented GLESv2 API\n");
}

//
// This function is called only once during initialiation before
// any thread has been created - hence it should NOT be thread safe.
//
bool gles2_dispatch_init(const char *path, GLESv2Dispatch *dispatch_table)
{
    const char *libName = getenv(glesv2_lib_env_var);
    if (!libName)
      libName = path;
    if (!libName)
      return false;

    char error[256];
    s_gles2_lib = emugl::SharedLibrary::open(libName, error, sizeof(error));
    if (!s_gles2_lib) {
        fprintf(stderr, "%s: Could not load %s [%s]\n", __FUNCTION__,
                libName, error);
        return false;
    }

    //
    // init the GLES dispatch table
    //
#define LOOKUP_SYMBOL(return_type,function_name,signature,callargs) \
    dispatch_table-> function_name = reinterpret_cast< function_name ## _t >( \
            s_gles2_lib->findSymbol(#function_name));

#define LOOKUP_EXT_SYMBOL(return_type,function_name,signature,callargs) \
    dispatch_table-> function_name = reinterpret_cast< function_name ## _t >( \
            s_egl.eglGetProcAddress(#function_name));

    LIST_GLES2_FUNCTIONS(LOOKUP_SYMBOL,LOOKUP_EXT_SYMBOL)

    dispatch_table->initialized = true;

    return true;
}

//
// This function is called only during initialization before
// any thread has been created - hence it should NOT be thread safe.
//
void *gles2_dispatch_get_proc_func(const char *name, void *userData)
{
    void* func = NULL;

    if (s_gles2_lib && !func) {
        func = (void *)s_gles2_lib->findSymbol(name);
    }

    if (!func) {
        func = (void *)s_egl.eglGetProcAddress(name);
    }

    // To make it consistent with the guest, redirect any unsupported functions
    // to gles2_unimplemented.
    if (!func) {
        func = (void *)gles2_unimplemented;
    }
    return func;
}
