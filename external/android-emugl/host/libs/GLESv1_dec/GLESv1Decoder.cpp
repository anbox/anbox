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
#include "GLESv1Decoder.h"

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void* SafePointerFromUInt(GLuint value) {
  return (void*)(uintptr_t)value;
}

GLESv1Decoder::GLESv1Decoder()
{
    m_contextData = NULL;
    m_glesDso = NULL;
}

GLESv1Decoder::~GLESv1Decoder()
{
    if (m_glesDso != NULL) {
        delete m_glesDso;
    }
}


int GLESv1Decoder::initGL(get_proc_func_t getProcFunc, void *getProcFuncData)
{
    this->initDispatchByName(getProcFunc, getProcFuncData);

    glGetCompressedTextureFormats = s_glGetCompressedTextureFormats;
    glVertexPointerOffset = s_glVertexPointerOffset;
    glColorPointerOffset = s_glColorPointerOffset;
    glNormalPointerOffset = s_glNormalPointerOffset;
    glTexCoordPointerOffset = s_glTexCoordPointerOffset;
    glPointSizePointerOffset = s_glPointSizePointerOffset;
    glWeightPointerOffset = s_glWeightPointerOffset;
    glMatrixIndexPointerOffset = s_glMatrixIndexPointerOffset;

    glVertexPointerData = s_glVertexPointerData;
    glColorPointerData = s_glColorPointerData;
    glNormalPointerData = s_glNormalPointerData;
    glTexCoordPointerData = s_glTexCoordPointerData;
    glPointSizePointerData = s_glPointSizePointerData;
    glWeightPointerData = s_glWeightPointerData;
    glMatrixIndexPointerData = s_glMatrixIndexPointerData;

    glDrawElementsOffset = s_glDrawElementsOffset;
    glDrawElementsData = s_glDrawElementsData;
    glFinishRoundTrip = s_glFinishRoundTrip;

    return 0;
}

int GLESv1Decoder::s_glFinishRoundTrip(void *self)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;
    ctx->glFinish();
    return 0;
}

void GLESv1Decoder::s_glVertexPointerOffset(void *self, GLint size, GLenum type, GLsizei stride, GLuint offset)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;
    ctx->glVertexPointer(size, type, stride, SafePointerFromUInt(offset));
}

void GLESv1Decoder::s_glColorPointerOffset(void *self, GLint size, GLenum type, GLsizei stride, GLuint offset)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;
    ctx->glColorPointer(size, type, stride, SafePointerFromUInt(offset));
}

void GLESv1Decoder::s_glTexCoordPointerOffset(void *self, GLint size, GLenum type, GLsizei stride, GLuint offset)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;
    ctx->glTexCoordPointer(size, type, stride, SafePointerFromUInt(offset));
}

void GLESv1Decoder::s_glNormalPointerOffset(void *self, GLenum type, GLsizei stride, GLuint offset)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;
    ctx->glNormalPointer(type, stride, SafePointerFromUInt(offset));
}

void GLESv1Decoder::s_glPointSizePointerOffset(void *self, GLenum type, GLsizei stride, GLuint offset)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;
    ctx->glPointSizePointerOES(type, stride, SafePointerFromUInt(offset));
}

void GLESv1Decoder::s_glWeightPointerOffset(void * self, GLint size, GLenum type, GLsizei stride, GLuint offset)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;
    ctx->glWeightPointerOES(size, type, stride, SafePointerFromUInt(offset));
}

void GLESv1Decoder::s_glMatrixIndexPointerOffset(void * self, GLint size, GLenum type, GLsizei stride, GLuint offset)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;
    ctx->glMatrixIndexPointerOES(size, type, stride, SafePointerFromUInt(offset));
}



#define STORE_POINTER_DATA_OR_ABORT(location)    \
    if (ctx->m_contextData != NULL) {   \
        ctx->m_contextData->storePointerData((location), data, datalen); \
    } else { \
        return; \
    }

void GLESv1Decoder::s_glVertexPointerData(void *self, GLint size, GLenum type, GLsizei stride, void *data, GLuint datalen)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;

    STORE_POINTER_DATA_OR_ABORT(GLDecoderContextData::VERTEX_LOCATION);

    ctx->glVertexPointer(size, type, 0, ctx->m_contextData->pointerData(GLDecoderContextData::VERTEX_LOCATION));
}

void GLESv1Decoder::s_glColorPointerData(void *self, GLint size, GLenum type, GLsizei stride, void *data, GLuint datalen)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;

    STORE_POINTER_DATA_OR_ABORT(GLDecoderContextData::COLOR_LOCATION);

    ctx->glColorPointer(size, type, 0, ctx->m_contextData->pointerData(GLDecoderContextData::COLOR_LOCATION));
}

void GLESv1Decoder::s_glTexCoordPointerData(void *self, GLint unit, GLint size, GLenum type, GLsizei stride, void *data, GLuint datalen)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;
    STORE_POINTER_DATA_OR_ABORT((GLDecoderContextData::PointerDataLocation)
                                (GLDecoderContextData::TEXCOORD0_LOCATION + unit));

    ctx->glTexCoordPointer(size, type, 0,
                           ctx->m_contextData->pointerData((GLDecoderContextData::PointerDataLocation)
                                                           (GLDecoderContextData::TEXCOORD0_LOCATION + unit)));
}

void GLESv1Decoder::s_glNormalPointerData(void *self, GLenum type, GLsizei stride, void *data, GLuint datalen)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;

    STORE_POINTER_DATA_OR_ABORT(GLDecoderContextData::NORMAL_LOCATION);

    ctx->glNormalPointer(type, 0, ctx->m_contextData->pointerData(GLDecoderContextData::NORMAL_LOCATION));
}

void GLESv1Decoder::s_glPointSizePointerData(void *self, GLenum type, GLsizei stride, void *data, GLuint datalen)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;

    STORE_POINTER_DATA_OR_ABORT(GLDecoderContextData::POINTSIZE_LOCATION);

    ctx->glPointSizePointerOES(type, 0, ctx->m_contextData->pointerData(GLDecoderContextData::POINTSIZE_LOCATION));
}

void GLESv1Decoder::s_glWeightPointerData(void * self, GLint size, GLenum type, GLsizei stride, void * data, GLuint datalen)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;

    STORE_POINTER_DATA_OR_ABORT(GLDecoderContextData::WEIGHT_LOCATION);

    ctx->glWeightPointerOES(size, type, 0, ctx->m_contextData->pointerData(GLDecoderContextData::WEIGHT_LOCATION));
}

void GLESv1Decoder::s_glMatrixIndexPointerData(void * self, GLint size, GLenum type, GLsizei stride, void * data, GLuint datalen)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;

    STORE_POINTER_DATA_OR_ABORT(GLDecoderContextData::MATRIXINDEX_LOCATION);

    ctx->glMatrixIndexPointerOES(size, type, 0, ctx->m_contextData->pointerData(GLDecoderContextData::MATRIXINDEX_LOCATION));
}

void GLESv1Decoder::s_glDrawElementsOffset(void *self, GLenum mode, GLsizei count, GLenum type, GLuint offset)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;
    ctx->glDrawElements(mode, count, type, SafePointerFromUInt(offset));
}

void GLESv1Decoder::s_glDrawElementsData(void *self, GLenum mode, GLsizei count, GLenum type, void * data, GLuint datalen)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)self;
    ctx->glDrawElements(mode, count, type, data);
}

void GLESv1Decoder::s_glGetCompressedTextureFormats(void *self, GLint count, GLint *data)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *) self;
    ctx->glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, data);
}

void *GLESv1Decoder::s_getProc(const char *name, void *userData)
{
    GLESv1Decoder *ctx = (GLESv1Decoder *)userData;

    if (ctx == NULL || ctx->m_glesDso == NULL) {
        return NULL;
    }

    void *func = NULL;
#ifdef USE_EGL_GETPROCADDRESS
    func = (void *) eglGetProcAddress(name);
#endif
    if (func == NULL) {
        func = (void *)(ctx->m_glesDso->findSymbol(name));
    }
    return func;
}
