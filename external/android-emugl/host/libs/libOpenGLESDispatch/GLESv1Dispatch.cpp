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
#include "OpenGLESDispatch/GLESv1Dispatch.h"
#include "OpenGLESDispatch/EGLDispatch.h"

#include <stdio.h>
#include <stdlib.h>

#include "emugl/common/shared_library.h"

extern EGLDispatch s_egl;

static emugl::SharedLibrary *s_gles1_lib = NULL;

static void gles1_unimplemented() {
    fprintf(stderr, "Called unimplemented GLESv1 API\n");
}

static void gles1_dummy() {}

#define ASSIGN_DUMMY(return_type, function_name, signature, call_args) do { \
    dispatch_table-> function_name = reinterpret_cast<function_name ## _t>(gles1_dummy); \
  } while(0);


#define LOOKUP_SYMBOL(return_type,function_name,signature,callargs) \
    dispatch_table-> function_name = reinterpret_cast< function_name ## _t >( \
            s_gles1_lib->findSymbol(#function_name));

#define LOOKUP_EXT_SYMBOL(return_type,function_name,signature,callargs) \
    dispatch_table-> function_name = reinterpret_cast< function_name ## _t >( \
            s_egl.eglGetProcAddress(#function_name));

namespace {
constexpr const char *glesv1_lib_env_var{"ANBOX_GLESv1_LIB"};
}

bool gles1_dispatch_init(const char *path, GLESv1Dispatch* dispatch_table) {
    if (!dispatch_table)
        return false;

    // If no path is given we assign dummy functions to all GL calls
    // we would have loaded from a real implementation.
    if (!path) {
        LIST_GLES1_FUNCTIONS(ASSIGN_DUMMY, ASSIGN_DUMMY);
        return true;
    }

    const char* libName = getenv(glesv1_lib_env_var);
    if (!libName)
      libName = path;
    if (!libName)
        return false;

    char error[256];
    s_gles1_lib = emugl::SharedLibrary::open(libName, error, sizeof(error));
    if (!s_gles1_lib) {
        fprintf(stderr, "%s: Could not load %s [%s]\n", __FUNCTION__,
                libName, error);
        return false;
    }

    LIST_GLES1_FUNCTIONS(LOOKUP_SYMBOL,LOOKUP_EXT_SYMBOL)

    dispatch_table->initialized = true;

    return true;
}

//
// This function is called only during initialization of the decoder before
// any thread has been created - hence it should NOT be thread safe.
//
void *gles1_dispatch_get_proc_func(const char *name, void *userData)
{
    void* func = NULL;
    if (s_gles1_lib && !func) {
        func = (void *)s_gles1_lib->findSymbol(name);
    }

    if (!func) {
        func = (void *)s_egl.eglGetProcAddress(name);
    }

    // To make it consistent with the guest, redirect any unsupported functions
    // to gles1_unimplemented.
    if (!func) {
        func = (void *)gles1_unimplemented;
    }
    return func;
}
