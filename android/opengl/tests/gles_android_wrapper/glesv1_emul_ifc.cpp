/*
* Copyright 2011 The Android Open Source Project
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

#include <stdlib.h>
#include "ApiInitializer.h"
#include <dlfcn.h>
#include "gl_wrapper_context.h"

extern "C" {
    gl_wrapper_context_t *createFromLib(void *solib, gl_wrapper_context_t *(*accessor)());
}

gl_wrapper_context_t * createFromLib(void *solib, gl_wrapper_context_t *(accessor)())
{
    gl_wrapper_context_t *ctx = new gl_wrapper_context_t;
    if (ctx == NULL) {
        return NULL;
    }
    ApiInitializer *initializer = new ApiInitializer(solib);
    ctx->initDispatchByName(ApiInitializer::s_getProc, initializer);
    gl_wrapper_context_t::setContextAccessor(accessor);
    delete initializer;
    return ctx;
}


