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
#ifndef _GL2_ENCODER_H_
#define _GL2_ENCODER_H_

#include "gl2_enc.h"
#include "GLClientState.h"
#include "GLSharedGroup.h"
#include "FixedBuffer.h"

class GL2Encoder : public gl2_encoder_context_t {
public:
    GL2Encoder(IOStream *stream, ChecksumCalculator* protocol);
    virtual ~GL2Encoder();
    void setClientState(GLClientState *state) {
        m_state = state;
    }
    void setSharedGroup(GLSharedGroupPtr shared){ m_shared = shared; }
    const GLClientState *state() { return m_state; }
    const GLSharedGroupPtr shared() { return m_shared; }
    void flush() { m_stream->flush(); }

    void setInitialized(){ m_initialized = true; };
    bool isInitialized(){ return m_initialized; };

    virtual void setError(GLenum error){ m_error = error; };
    virtual GLenum getError() { return m_error; };

    void override2DTextureTarget(GLenum target);
    void restore2DTextureTarget();

private:

    bool    m_initialized;
    GLClientState *m_state;
    GLSharedGroupPtr m_shared;
    GLenum  m_error;

    GLint *m_compressedTextureFormats;
    GLint m_num_compressedTextureFormats;
    GLint *getCompressedTextureFormats();

    GLint m_max_cubeMapTextureSize;
    GLint m_max_renderBufferSize;
    GLint m_max_textureSize;
    FixedBuffer m_fixedBuffer;

    void sendVertexAttributes(GLint first, GLsizei count);
    bool updateHostTexture2DBinding(GLenum texUnit, GLenum newTarget);
    void checkValidUniformParam(void * self, GLsizei count, GLboolean transpose);
    void getHostLocation(void *self, GLint location, GLint *hostLoc);


    glGetError_client_proc_t    m_glGetError_enc;
    static GLenum s_glGetError(void * self);

    glFlush_client_proc_t m_glFlush_enc;
    static void s_glFlush(void * self);

    glPixelStorei_client_proc_t m_glPixelStorei_enc;
    static void s_glPixelStorei(void *self, GLenum param, GLint value);

    glGetString_client_proc_t m_glGetString_enc;
    static const GLubyte * s_glGetString(void *self, GLenum name);

    glBindBuffer_client_proc_t m_glBindBuffer_enc;
    static void s_glBindBuffer(void *self, GLenum target, GLuint id);


    glBufferData_client_proc_t m_glBufferData_enc;
    static void s_glBufferData(void *self, GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage);
    glBufferSubData_client_proc_t m_glBufferSubData_enc;
    static void s_glBufferSubData(void *self, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data);
    glDeleteBuffers_client_proc_t m_glDeleteBuffers_enc;
    static void s_glDeleteBuffers(void *self, GLsizei n, const GLuint * buffers);

    glDrawArrays_client_proc_t m_glDrawArrays_enc;
    static void s_glDrawArrays(void *self, GLenum mode, GLint first, GLsizei count);

    glDrawElements_client_proc_t m_glDrawElements_enc;
    static void s_glDrawElements(void *self, GLenum mode, GLsizei count, GLenum type, const void *indices);


    glGetIntegerv_client_proc_t m_glGetIntegerv_enc;
    static void s_glGetIntegerv(void *self, GLenum pname, GLint *ptr);

    glGetFloatv_client_proc_t m_glGetFloatv_enc;
    static void s_glGetFloatv(void *self, GLenum pname, GLfloat *ptr);

    glGetBooleanv_client_proc_t m_glGetBooleanv_enc;
    static void s_glGetBooleanv(void *self, GLenum pname, GLboolean *ptr);

    glVertexAttribPointer_client_proc_t m_glVertexAttribPointer_enc;
    static void s_glVertexAttribPointer(void *self, GLuint indx, GLint size, GLenum type,
                                        GLboolean normalized, GLsizei stride, const GLvoid * ptr);

    glEnableVertexAttribArray_client_proc_t m_glEnableVertexAttribArray_enc;
    static void s_glEnableVertexAttribArray(void *self, GLuint index);

    glDisableVertexAttribArray_client_proc_t m_glDisableVertexAttribArray_enc;
    static void s_glDisableVertexAttribArray(void *self, GLuint index);

    glGetVertexAttribiv_client_proc_t m_glGetVertexAttribiv_enc;
    static void s_glGetVertexAttribiv(void *self, GLuint index, GLenum pname, GLint *params);

    glGetVertexAttribfv_client_proc_t m_glGetVertexAttribfv_enc;
    static void s_glGetVertexAttribfv(void *self, GLuint index, GLenum pname, GLfloat *params);

    glGetVertexAttribPointerv_client_proc_t m_glGetVertexAttribPointerv_enc;
    static void s_glGetVertexAttribPointerv(void *self, GLuint index, GLenum pname, GLvoid **pointer);

    static void s_glShaderBinary(void *self, GLsizei n, const GLuint *shaders, GLenum binaryformat, const void* binary, GLsizei length);

    static void s_glShaderSource(void *self, GLuint shader, GLsizei count, const GLchar * const *string, const GLint *length);

    static void s_glFinish(void *self);

    glLinkProgram_client_proc_t m_glLinkProgram_enc;
    static void s_glLinkProgram(void *self, GLuint program);

    glDeleteProgram_client_proc_t m_glDeleteProgram_enc;
    static void s_glDeleteProgram(void * self, GLuint program);

    glGetUniformiv_client_proc_t m_glGetUniformiv_enc;
    static void s_glGetUniformiv(void *self, GLuint program, GLint location , GLint *params);

    glGetUniformfv_client_proc_t m_glGetUniformfv_enc;
    static void s_glGetUniformfv(void *self, GLuint program, GLint location , GLfloat *params);

    glCreateProgram_client_proc_t m_glCreateProgram_enc;
    static GLuint s_glCreateProgram(void *self);

    glCreateShader_client_proc_t m_glCreateShader_enc;
    static GLuint s_glCreateShader(void *self, GLenum shaderType);

    glDeleteShader_client_proc_t m_glDeleteShader_enc;
    static void s_glDeleteShader(void *self, GLuint shader);

    glAttachShader_client_proc_t m_glAttachShader_enc;
    static void s_glAttachShader(void *self, GLuint program, GLuint shader);

    glDetachShader_client_proc_t m_glDetachShader_enc;
    static void s_glDetachShader(void *self, GLuint program, GLuint shader);

    glGetAttachedShaders_client_proc_t m_glGetAttachedShaders_enc;
    static void s_glGetAttachedShaders(void *self, GLuint program, GLsizei maxCount,
            GLsizei* count, GLuint* shaders);

    glGetShaderSource_client_proc_t m_glGetShaderSource_enc;
    static void s_glGetShaderSource(void *self, GLuint shader, GLsizei bufsize,
            GLsizei* length, GLchar* source);

    glGetShaderInfoLog_client_proc_t m_glGetShaderInfoLog_enc;
    static void s_glGetShaderInfoLog(void *self,GLuint shader,
            GLsizei bufsize, GLsizei* length, GLchar* infolog);

    glGetProgramInfoLog_client_proc_t m_glGetProgramInfoLog_enc;
    static void s_glGetProgramInfoLog(void *self,GLuint program,
            GLsizei bufsize, GLsizei* length, GLchar* infolog);

    glGetUniformLocation_client_proc_t m_glGetUniformLocation_enc;
    static int s_glGetUniformLocation(void *self, GLuint program, const GLchar *name);
    glUseProgram_client_proc_t m_glUseProgram_enc;

    glUniform1f_client_proc_t m_glUniform1f_enc;
    glUniform1fv_client_proc_t m_glUniform1fv_enc;
    glUniform1i_client_proc_t m_glUniform1i_enc;
    glUniform1iv_client_proc_t m_glUniform1iv_enc;
    glUniform2f_client_proc_t m_glUniform2f_enc;
    glUniform2fv_client_proc_t m_glUniform2fv_enc;
    glUniform2i_client_proc_t m_glUniform2i_enc;
    glUniform2iv_client_proc_t m_glUniform2iv_enc;
    glUniform3f_client_proc_t m_glUniform3f_enc;
    glUniform3fv_client_proc_t m_glUniform3fv_enc;
    glUniform3i_client_proc_t m_glUniform3i_enc;
    glUniform3iv_client_proc_t m_glUniform3iv_enc;
    glUniform4f_client_proc_t m_glUniform4f_enc;
    glUniform4fv_client_proc_t m_glUniform4fv_enc;
    glUniform4i_client_proc_t m_glUniform4i_enc;
    glUniform4iv_client_proc_t m_glUniform4iv_enc;
    glUniformMatrix2fv_client_proc_t m_glUniformMatrix2fv_enc;
    glUniformMatrix3fv_client_proc_t m_glUniformMatrix3fv_enc;
    glUniformMatrix4fv_client_proc_t m_glUniformMatrix4fv_enc;

    static void s_glUseProgram(void *self, GLuint program);
	static void s_glUniform1f(void *self , GLint location, GLfloat x);
	static void s_glUniform1fv(void *self , GLint location, GLsizei count, const GLfloat* v);
	static void s_glUniform1i(void *self , GLint location, GLint x);
	static void s_glUniform1iv(void *self , GLint location, GLsizei count, const GLint* v);
	static void s_glUniform2f(void *self , GLint location, GLfloat x, GLfloat y);
	static void s_glUniform2fv(void *self , GLint location, GLsizei count, const GLfloat* v);
	static void s_glUniform2i(void *self , GLint location, GLint x, GLint y);
	static void s_glUniform2iv(void *self , GLint location, GLsizei count, const GLint* v);
	static void s_glUniform3f(void *self , GLint location, GLfloat x, GLfloat y, GLfloat z);
	static void s_glUniform3fv(void *self , GLint location, GLsizei count, const GLfloat* v);
	static void s_glUniform3i(void *self , GLint location, GLint x, GLint y, GLint z);
	static void s_glUniform3iv(void *self , GLint location, GLsizei count, const GLint* v);
	static void s_glUniform4f(void *self , GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	static void s_glUniform4fv(void *self , GLint location, GLsizei count, const GLfloat* v);
	static void s_glUniform4i(void *self , GLint location, GLint x, GLint y, GLint z, GLint w);
	static void s_glUniform4iv(void *self , GLint location, GLsizei count, const GLint* v);
	static void s_glUniformMatrix2fv(void *self , GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static void s_glUniformMatrix3fv(void *self , GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static void s_glUniformMatrix4fv(void *self , GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

    glActiveTexture_client_proc_t m_glActiveTexture_enc;
    glBindTexture_client_proc_t m_glBindTexture_enc;
    glDeleteTextures_client_proc_t m_glDeleteTextures_enc;
    glGetTexParameterfv_client_proc_t m_glGetTexParameterfv_enc;
    glGetTexParameteriv_client_proc_t m_glGetTexParameteriv_enc;
    glTexParameterf_client_proc_t m_glTexParameterf_enc;
    glTexParameterfv_client_proc_t m_glTexParameterfv_enc;
    glTexParameteri_client_proc_t m_glTexParameteri_enc;
    glTexParameteriv_client_proc_t m_glTexParameteriv_enc;
    glTexImage2D_client_proc_t m_glTexImage2D_enc;
    glTexSubImage2D_client_proc_t m_glTexSubImage2D_enc;

    static void s_glActiveTexture(void* self, GLenum texture);
    static void s_glBindTexture(void* self, GLenum target, GLuint texture);
    static void s_glDeleteTextures(void* self, GLsizei n, const GLuint* textures);
    static void s_glGetTexParameterfv(void* self, GLenum target, GLenum pname, GLfloat* params);
    static void s_glGetTexParameteriv(void* self, GLenum target, GLenum pname, GLint* params);
    static void s_glTexParameterf(void* self, GLenum target, GLenum pname, GLfloat param);
    static void s_glTexParameterfv(void* self, GLenum target, GLenum pname, const GLfloat* params);
    static void s_glTexParameteri(void* self, GLenum target, GLenum pname, GLint param);
    static void s_glTexParameteriv(void* self, GLenum target, GLenum pname, const GLint* params);
    static void s_glTexImage2D(void* self, GLenum target, GLint level, GLint internalformat,
            GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
            const GLvoid* pixels);
    static void s_glTexSubImage2D(void* self, GLenum target, GLint level, GLint xoffset,
            GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type,
            const GLvoid* pixels);

public:
    glEGLImageTargetTexture2DOES_client_proc_t m_glEGLImageTargetTexture2DOES_enc;

};
#endif
