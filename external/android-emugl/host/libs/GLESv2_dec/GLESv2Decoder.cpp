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

#include "GLESv2Decoder.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

static inline void* SafePointerFromUInt(GLuint value) {
  return (void*)(uintptr_t)value;
}

GLESv2Decoder::GLESv2Decoder()
{
    m_contextData = NULL;
    m_GL2library = NULL;
}

GLESv2Decoder::~GLESv2Decoder()
{
    delete m_GL2library;
}

void *GLESv2Decoder::s_getProc(const char *name, void *userData)
{
    GLESv2Decoder *ctx = (GLESv2Decoder *) userData;

    if (ctx == NULL || ctx->m_GL2library == NULL) {
        return NULL;
    }

    void *func = NULL;
#ifdef USE_EGL_GETPROCADDRESS
    func = (void *) eglGetProcAddress(name);
#endif
    if (func == NULL) {
        func = (void *) ctx->m_GL2library->findSymbol(name);
    }
    return func;
}

int GLESv2Decoder::initGL(get_proc_func_t getProcFunc, void *getProcFuncData)
{
    this->initDispatchByName(getProcFunc, getProcFuncData);

    glGetCompressedTextureFormats = s_glGetCompressedTextureFormats;
    glVertexAttribPointerData = s_glVertexAttribPointerData;
    glVertexAttribPointerOffset = s_glVertexAttribPointerOffset;

    glDrawElementsOffset = s_glDrawElementsOffset;
    glDrawElementsData = s_glDrawElementsData;
    glShaderString = s_glShaderString;
    glFinishRoundTrip = s_glFinishRoundTrip;
    return 0;

}

int GLESv2Decoder::s_glFinishRoundTrip(void *self)
{
    GLESv2Decoder *ctx = (GLESv2Decoder *)self;
    ctx->glFinish();
    return 0;
}

void GLESv2Decoder::s_glGetCompressedTextureFormats(void *self, int count, GLint *formats)
{
    GLESv2Decoder *ctx = (GLESv2Decoder *) self;

    int nFormats;
    ctx->glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &nFormats);
    if (nFormats > count) {
        fprintf(stderr, "%s: GetCompressedTextureFormats: The requested number of formats does not match the number that is reported by OpenGL\n", __FUNCTION__);
    } else {
        ctx->glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, formats);
    }
}

void GLESv2Decoder::s_glVertexAttribPointerData(void *self, GLuint indx, GLint size, GLenum type,
                                             GLboolean normalized, GLsizei stride,  void * data, GLuint datalen)
{
    GLESv2Decoder *ctx = (GLESv2Decoder *) self;
    if (ctx->m_contextData != NULL) {
        ctx->m_contextData->storePointerData(indx, data, datalen);
        // note - the stride of the data is always zero when it comes out of the codec.
        // See gl2.attrib for the packing function call.
        ctx->glVertexAttribPointer(indx, size, type, normalized, 0, ctx->m_contextData->pointerData(indx));
    }
}

void GLESv2Decoder::s_glVertexAttribPointerOffset(void *self, GLuint indx, GLint size, GLenum type,
                                               GLboolean normalized, GLsizei stride,  GLuint data)
{
    GLESv2Decoder *ctx = (GLESv2Decoder *) self;
    ctx->glVertexAttribPointer(indx, size, type, normalized, stride, SafePointerFromUInt(data));
}


void GLESv2Decoder::s_glDrawElementsData(void *self, GLenum mode, GLsizei count, GLenum type, void * data, GLuint datalen)
{
    GLESv2Decoder *ctx = (GLESv2Decoder *)self;
    ctx->glDrawElements(mode, count, type, data);
}


void GLESv2Decoder::s_glDrawElementsOffset(void *self, GLenum mode, GLsizei count, GLenum type, GLuint offset)
{
    GLESv2Decoder *ctx = (GLESv2Decoder *)self;
    ctx->glDrawElements(mode, count, type, SafePointerFromUInt(offset));
}

void GLESv2Decoder::s_glShaderString(void *self, GLuint shader, const GLchar* string, GLsizei len)
{
    GLESv2Decoder *ctx = (GLESv2Decoder *)self;
    ctx->glShaderSource(shader, 1, &string, NULL);
}
