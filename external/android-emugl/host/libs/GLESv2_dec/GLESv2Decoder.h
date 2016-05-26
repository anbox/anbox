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

#ifndef _GL2_DECODER_H_
#define _GL2_DECODER_H_

#include "gles2_dec.h"
#include "GLDecoderContextData.h"
#include "emugl/common/shared_library.h"

class GLESv2Decoder : public gles2_decoder_context_t
{
public:
    typedef void *(*get_proc_func_t)(const char *name, void *userData);
    GLESv2Decoder();
    ~GLESv2Decoder();
    int initGL(get_proc_func_t getProcFunc, void *getProcFuncData);
    void setContextData(GLDecoderContextData *contextData) { m_contextData = contextData; }
private:
    GLDecoderContextData *m_contextData;
    emugl::SharedLibrary* m_GL2library;

    static void *s_getProc(const char *name, void *userData);
    static void gles2_APIENTRY s_glGetCompressedTextureFormats(void *self, int count, GLint *formats);
    static void gles2_APIENTRY s_glVertexAttribPointerData(void *self, GLuint indx, GLint size, GLenum type,
                                      GLboolean normalized, GLsizei stride,  void * data, GLuint datalen);
    static void gles2_APIENTRY s_glVertexAttribPointerOffset(void *self, GLuint indx, GLint size, GLenum type,
                                        GLboolean normalized, GLsizei stride,  GLuint offset);

    static void gles2_APIENTRY s_glDrawElementsOffset(void *self, GLenum mode, GLsizei count, GLenum type, GLuint offset);
    static void gles2_APIENTRY s_glDrawElementsData(void *self, GLenum mode, GLsizei count, GLenum type, void * data, GLuint datalen);
    static void gles2_APIENTRY s_glShaderString(void *self, GLuint shader, const GLchar* string, GLsizei len);
    static int  gles2_APIENTRY s_glFinishRoundTrip(void *self);
};
#endif
