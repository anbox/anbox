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
#include "ClientAPIExts.h"
#include "ThreadInfo.h"
#include <GLES/gl.h>
#include <GLES/glext.h>
#include "eglContext.h"

namespace ClientAPIExts
{

//
// define function pointer type for each extention function
// typename has the form __egl_{funcname}_t
//
#define FUNC_TYPE(fname) __egl_ ## fname ## _t
#define API_ENTRY(fname,params,args) \
    typedef void (GL_APIENTRY *FUNC_TYPE(fname)) params;

#define API_ENTRY_RET(rtype,fname,params,args) \
    typedef rtype (GL_APIENTRY *FUNC_TYPE(fname)) params;

#include "ClientAPIExts.in"
#undef API_ENTRY
#undef API_ENTRY_RET

/////
// Define static table to store the function value for each
// client API. functions pointers will get initialized through
// ClientAPIExts::initClientFuncs function after each client API has been
// loaded.
/////
#define API_ENTRY(fname,params,args) \
    FUNC_TYPE(fname) fname;

#define API_ENTRY_RET(rtype,fname,params,args) \
    API_ENTRY(fname,params,args)

static struct _ext_table
{
#include "ClientAPIExts.in"
} s_client_extensions[2];

#undef API_ENTRY
#undef API_ENTRY_RET

//
// This function initialized each entry in the s_client_extensions
// struct at the givven index using the givven client interface
//
void initClientFuncs(const EGLClient_glesInterface *iface, int idx)
{
#define API_ENTRY(fname,params,args) \
    s_client_extensions[idx].fname = \
          (FUNC_TYPE(fname))iface->getProcAddress(#fname);

#define API_ENTRY_RET(rtype,fname,params,args) \
    API_ENTRY(fname,params,args)

    //
    // reset all func pointers to NULL
    //
    memset(&s_client_extensions[idx], 0, sizeof(struct _ext_table));

    //
    // And now query the GLES library for each proc address
    //
#include "ClientAPIExts.in"
#undef API_ENTRY
#undef API_ENTRY_RET
}

//
// Define implementation for each extension function which checks
// the current context version and calls to the correct client API
// function.
//
#define API_ENTRY(fname,params,args) \
    static void _egl_ ## fname params \
    { \
        EGLThreadInfo* thread  = getEGLThreadInfo(); \
        if (!thread->currentContext) { \
            return; \
        } \
        int idx = (int)thread->currentContext->version - 1; \
        if (!s_client_extensions[idx].fname) { \
            return; \
        } \
        (*s_client_extensions[idx].fname) args; \
    }

#define API_ENTRY_RET(rtype,fname,params,args) \
    static rtype _egl_ ## fname params \
    { \
        EGLThreadInfo* thread  = getEGLThreadInfo(); \
        if (!thread->currentContext) { \
            return (rtype)0; \
        } \
        int idx = (int)thread->currentContext->version - 1; \
        if (!s_client_extensions[idx].fname) { \
            return (rtype)0; \
        } \
        return (*s_client_extensions[idx].fname) args; \
    }

#include "ClientAPIExts.in"
#undef API_ENTRY
#undef API_ENTRY_RET

//
// Define a table to map function names to the local _egl_ version of
// the extension function, to be used in eglGetProcAddress.
//
#define API_ENTRY(fname,params,args) \
    { #fname, (void*)_egl_ ## fname},
#define API_ENTRY_RET(rtype,fname,params,args) \
    API_ENTRY(fname,params,args)

static const struct _client_ext_funcs {
    const char *fname;
    void* proc;
} s_client_ext_funcs[] = {
#include "ClientAPIExts.in"
};
static const int numExtFuncs = sizeof(s_client_ext_funcs) / 
                               sizeof(s_client_ext_funcs[0]);

#undef API_ENTRY
#undef API_ENTRY_RET

//
// returns the __egl_ version of the givven extension function name.
//
void* getProcAddress(const char *fname)
{
    for (int i=0; i<numExtFuncs; i++) {
        if (!strcmp(fname, s_client_ext_funcs[i].fname)) {
            return s_client_ext_funcs[i].proc;
        }
    }
    return NULL;
}

} // of namespace ClientAPIExts
