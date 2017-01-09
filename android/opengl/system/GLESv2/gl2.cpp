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

#include "EGLClientIface.h"
#include "HostConnection.h"
#include "GL2Encoder.h"
#include "GLES/gl.h"
#include "GLES/glext.h"
#include "ErrorLog.h"
#include "gralloc_cb.h"
#include "ThreadInfo.h"
#include "EGLImage.h"

//XXX: fix this macro to get the context from fast tls path
#define GET_CONTEXT GL2Encoder * ctx = getEGLThreadInfo()->hostConn->gl2Encoder();

#include "gl2_entry.cpp"

//The functions table
#include "gl2_ftable.h"


static EGLClient_eglInterface * s_egl = NULL;
static EGLClient_glesInterface * s_gl = NULL;

#define DEFINE_AND_VALIDATE_HOST_CONNECTION(ret) \
    HostConnection *hostCon = HostConnection::get(); \
    if (!hostCon) { \
        ALOGE("egl: Failed to get host connection\n"); \
        return ret; \
    } \
    renderControl_encoder_context_t *rcEnc = hostCon->rcEncoder(); \
    if (!rcEnc) { \
        ALOGE("egl: Failed to get renderControl encoder context\n"); \
        return ret; \
    }

//GL extensions
void glEGLImageTargetTexture2DOES(void * self, GLenum target, GLeglImageOES img)
{
    (void)self;
    (void)target;

    DBG("glEGLImageTargetTexture2DOES v2 target=%#x img=%p\n", target, img);

    EGLImage_t *image = (EGLImage_t*)img;

    if (image->target == EGL_NATIVE_BUFFER_ANDROID) {
        //TODO: check error - we don't have a way to set gl error
        android_native_buffer_t* native_buffer = image->native_buffer;

        if (native_buffer->common.magic != ANDROID_NATIVE_BUFFER_MAGIC) {
            return;
        }

        if (native_buffer->common.version != sizeof(android_native_buffer_t)) {
            return;
        }

        GET_CONTEXT;
        DEFINE_AND_VALIDATE_HOST_CONNECTION();

        ctx->override2DTextureTarget(target);
        rcEnc->rcBindTexture(rcEnc, ((cb_handle_t *)(native_buffer->handle))->hostHandle);
        ctx->restore2DTextureTarget();
    }
    else if (image->target == EGL_GL_TEXTURE_2D_KHR) {
        GET_CONTEXT;
        ctx->override2DTextureTarget(target);
        GLeglImageOES hostImage = reinterpret_cast<GLeglImageOES>((intptr_t)image->host_egl_image);
        ctx->m_glEGLImageTargetTexture2DOES_enc(self, target, hostImage);
        ctx->restore2DTextureTarget();
    }
}

void glEGLImageTargetRenderbufferStorageOES(void *self, GLenum target, GLeglImageOES image)
{
    (void)self;
    (void)target;

    DBG("glEGLImageTargetRenderbufferStorageOES v2 image=%p\n", image);
    //TODO: check error - we don't have a way to set gl error
    android_native_buffer_t* native_buffer = (android_native_buffer_t*)image;

    if (native_buffer->common.magic != ANDROID_NATIVE_BUFFER_MAGIC) {
        return;
    }

    if (native_buffer->common.version != sizeof(android_native_buffer_t)) {
        return;
    }

    DEFINE_AND_VALIDATE_HOST_CONNECTION();
    rcEnc->rcBindRenderbuffer(rcEnc, ((cb_handle_t *)(native_buffer->handle))->hostHandle);

    return;
}

void * getProcAddress(const char * procname)
{
    // search in GL function table
    for (int i=0; i<gl2_num_funcs; i++) {
        if (!strcmp(gl2_funcs_by_name[i].name, procname)) {
            return gl2_funcs_by_name[i].proc;
        }
    }
    return NULL;
}

void finish()
{
    glFinish();
}

const GLubyte *my_glGetString (void *self, GLenum name)
{
    (void)self;

    //see ref in https://www.khronos.org/opengles/sdk/docs/man
    //name in glGetString can be one of the following five values
    switch (name) {
        case GL_VERSION:
        case GL_VENDOR:
        case GL_RENDERER:
        case GL_SHADING_LANGUAGE_VERSION:
        case GL_EXTENSIONS:
            if (s_egl) {
                return (const GLubyte*)s_egl->getGLString(name);
            }
            break;
        default:
            GET_CONTEXT;
            ctx->setError(GL_INVALID_ENUM);
            break;
    }
    return NULL;
}

void init()
{
    GET_CONTEXT;
    ctx->m_glEGLImageTargetTexture2DOES_enc = ctx->glEGLImageTargetTexture2DOES;
    ctx->glEGLImageTargetTexture2DOES = &glEGLImageTargetTexture2DOES;
    ctx->glEGLImageTargetRenderbufferStorageOES = &glEGLImageTargetRenderbufferStorageOES;
    ctx->glGetString = &my_glGetString;
}

extern "C" {
EGLClient_glesInterface * init_emul_gles(EGLClient_eglInterface *eglIface)
{
    s_egl = eglIface;

    if (!s_gl) {
        s_gl = new EGLClient_glesInterface();
        s_gl->getProcAddress = getProcAddress;
        s_gl->finish = finish;
        s_gl->init = init;
    }

    return s_gl;
}
} //extern


