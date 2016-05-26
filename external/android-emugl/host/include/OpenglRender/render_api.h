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
#pragma once

#include "OpenglRender/render_api_functions.h"

#include <KHR/khrplatform.h>

/* This header and its declarations must be usable from C code.
 *
 * If RENDER_API_NO_PROTOTYPES is #defined before including this header, only
 * the interface function pointer types will be declared, not the prototypes.
 * This allows the client to use those names for its function pointer variables.
 *
 * All interfaces which can fail return an int, with zero indicating failure
 * and anything else indicating success.
 */

#ifdef __cplusplus
extern "C" {
#endif

// Use KHRONOS_APICALL to control visibility, but do not use KHRONOS_APIENTRY
// because we don't need the functions to be __stdcall on Win32.
#define RENDER_APICALL  KHRONOS_APICALL
#define RENDER_APIENTRY

#define RENDER_API_DECLARE(return_type, func_name, signature, callargs) \
    typedef return_type (RENDER_APIENTRY *func_name ## Fn) signature; \
    RENDER_APICALL return_type RENDER_APIENTRY func_name signature;

typedef void (*emugl_logger_func_t)(const char* fmt, ...);

typedef struct {
    emugl_logger_func_t coarse;
    emugl_logger_func_t fine;
} emugl_logger_struct;

LIST_RENDER_API_FUNCTIONS(RENDER_API_DECLARE)

#ifdef __cplusplus
}
#endif
