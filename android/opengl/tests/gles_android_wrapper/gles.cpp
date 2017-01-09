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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gles_dispatch.h"
#include "gles_ftable.h"
#include <EGL/egl.h>
#include <cutils/log.h>

static struct gles_dispatch *s_dispatch = NULL;

void init_gles(void *gles_android)
{
    s_dispatch = create_gles_dispatch(gles_android);
    if (s_dispatch == NULL) {
        ALOGE("failed to create gles dispatch\n");
    }
}

static struct gles_dispatch *getDispatch()
{
    if (!s_dispatch) {
        fprintf(stderr,"FATAL ERROR: GLES has not been initialized\n");
        exit(-1);
    }

    return s_dispatch;
}

__eglMustCastToProperFunctionPointerType gles_getProcAddress(const char *procname)
{
     for (int i=0; i<gles_num_funcs; i++) {
         if (!strcmp(gles_funcs_by_name[i].name, procname)) {
             return (__eglMustCastToProperFunctionPointerType)gles_funcs_by_name[i].proc;
         }
     }

     return NULL;
}

///////////// Path-through functions ///////////////
void glAlphaFunc(GLenum func, GLclampf ref)
{
     getDispatch()->glAlphaFunc(func, ref);
}

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
     getDispatch()->glClearColor(red, green, blue, alpha);
}

void glClearDepthf(GLclampf depth)
{
     getDispatch()->glClearDepthf(depth);
}

void glClipPlanef(GLenum plane, const GLfloat *equation)
{
     getDispatch()->glClipPlanef(plane, equation);
}

void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
     getDispatch()->glColor4f(red, green, blue, alpha);
}

void glDepthRangef(GLclampf zNear, GLclampf zFar)
{
     getDispatch()->glDepthRangef(zNear, zFar);
}

void glFogf(GLenum pname, GLfloat param)
{
     getDispatch()->glFogf(pname, param);
}

void glFogfv(GLenum pname, const GLfloat *params)
{
     getDispatch()->glFogfv(pname, params);
}

void glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
     getDispatch()->glFrustumf(left, right, bottom, top, zNear, zFar);
}

void glGetClipPlanef(GLenum pname, GLfloat eqn[4])
{
     getDispatch()->glGetClipPlanef(pname, eqn);
}

void glGetFloatv(GLenum pname, GLfloat *params)
{
     getDispatch()->glGetFloatv(pname, params);
}

void glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
     getDispatch()->glGetLightfv(light, pname, params);
}

void glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
     getDispatch()->glGetMaterialfv(face, pname, params);
}

void glGetTexEnvfv(GLenum env, GLenum pname, GLfloat *params)
{
     getDispatch()->glGetTexEnvfv(env, pname, params);
}

void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
     getDispatch()->glGetTexParameterfv(target, pname, params);
}

void glLightModelf(GLenum pname, GLfloat param)
{
     getDispatch()->glLightModelf(pname, param);
}

void glLightModelfv(GLenum pname, const GLfloat *params)
{
     getDispatch()->glLightModelfv(pname, params);
}

void glLightf(GLenum light, GLenum pname, GLfloat param)
{
     getDispatch()->glLightf(light, pname, param);
}

void glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
     getDispatch()->glLightfv(light, pname, params);
}

void glLineWidth(GLfloat width)
{
     getDispatch()->glLineWidth(width);
}

void glLoadMatrixf(const GLfloat *m)
{
     getDispatch()->glLoadMatrixf(m);
}

void glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
     getDispatch()->glMaterialf(face, pname, param);
}

void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
     getDispatch()->glMaterialfv(face, pname, params);
}

void glMultMatrixf(const GLfloat *m)
{
     getDispatch()->glMultMatrixf(m);
}

void glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
     getDispatch()->glMultiTexCoord4f(target, s, t, r, q);
}

void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
     getDispatch()->glNormal3f(nx, ny, nz);
}

void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
     getDispatch()->glOrthof(left, right, bottom, top, zNear, zFar);
}

void glPointParameterf(GLenum pname, GLfloat param)
{
     getDispatch()->glPointParameterf(pname, param);
}

void glPointParameterfv(GLenum pname, const GLfloat *params)
{
     getDispatch()->glPointParameterfv(pname, params);
}

void glPointSize(GLfloat size)
{
     getDispatch()->glPointSize(size);
}

void glPolygonOffset(GLfloat factor, GLfloat units)
{
     getDispatch()->glPolygonOffset(factor, units);
}

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
     getDispatch()->glRotatef(angle, x, y, z);
}

void glScalef(GLfloat x, GLfloat y, GLfloat z)
{
     getDispatch()->glScalef(x, y, z);
}

void glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
     getDispatch()->glTexEnvf(target, pname, param);
}

void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
     getDispatch()->glTexEnvfv(target, pname, params);
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
     getDispatch()->glTexParameterf(target, pname, param);
}

void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
     getDispatch()->glTexParameterfv(target, pname, params);
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
     getDispatch()->glTranslatef(x, y, z);
}

void glActiveTexture(GLenum texture)
{
     getDispatch()->glActiveTexture(texture);
}

void glAlphaFuncx(GLenum func, GLclampx ref)
{
     getDispatch()->glAlphaFuncx(func, ref);
}

void glBindBuffer(GLenum target, GLuint buffer)
{
     getDispatch()->glBindBuffer(target, buffer);
}

void glBindTexture(GLenum target, GLuint texture)
{
     getDispatch()->glBindTexture(target, texture);
}

void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
     getDispatch()->glBlendFunc(sfactor, dfactor);
}

void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
     getDispatch()->glBufferData(target, size, data, usage);
}

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
     getDispatch()->glBufferSubData(target, offset, size, data);
}

void glClear(GLbitfield mask)
{
     getDispatch()->glClear(mask);
}

void glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha)
{
     getDispatch()->glClearColorx(red, green, blue, alpha);
}

void glClearDepthx(GLclampx depth)
{
     getDispatch()->glClearDepthx(depth);
}

void glClearStencil(GLint s)
{
     getDispatch()->glClearStencil(s);
}

void glClientActiveTexture(GLenum texture)
{
     getDispatch()->glClientActiveTexture(texture);
}

void glClipPlanex(GLenum plane, const GLfixed *equation)
{
     getDispatch()->glClipPlanex(plane, equation);
}

void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
     getDispatch()->glColor4ub(red, green, blue, alpha);
}

void glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
     getDispatch()->glColor4x(red, green, blue, alpha);
}

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
     getDispatch()->glColorMask(red, green, blue, alpha);
}

void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
     getDispatch()->glColorPointer(size, type, stride, pointer);
}

void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
     getDispatch()->glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
     getDispatch()->glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
     getDispatch()->glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
     getDispatch()->glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

void glCullFace(GLenum mode)
{
     getDispatch()->glCullFace(mode);
}

void glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
     getDispatch()->glDeleteBuffers(n, buffers);
}

void glDeleteTextures(GLsizei n, const GLuint *textures)
{
     getDispatch()->glDeleteTextures(n, textures);
}

void glDepthFunc(GLenum func)
{
     getDispatch()->glDepthFunc(func);
}

void glDepthMask(GLboolean flag)
{
     getDispatch()->glDepthMask(flag);
}

void glDepthRangex(GLclampx zNear, GLclampx zFar)
{
     getDispatch()->glDepthRangex(zNear, zFar);
}

void glDisable(GLenum cap)
{
     getDispatch()->glDisable(cap);
}

void glDisableClientState(GLenum array)
{
     getDispatch()->glDisableClientState(array);
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
     getDispatch()->glDrawArrays(mode, first, count);
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
     getDispatch()->glDrawElements(mode, count, type, indices);
}

void glEnable(GLenum cap)
{
     getDispatch()->glEnable(cap);
}

void glEnableClientState(GLenum array)
{
     getDispatch()->glEnableClientState(array);
}

void glFinish()
{
     getDispatch()->glFinish();
}

void glFlush()
{
     getDispatch()->glFlush();
}

void glFogx(GLenum pname, GLfixed param)
{
     getDispatch()->glFogx(pname, param);
}

void glFogxv(GLenum pname, const GLfixed *params)
{
     getDispatch()->glFogxv(pname, params);
}

void glFrontFace(GLenum mode)
{
     getDispatch()->glFrontFace(mode);
}

void glFrustumx(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
     getDispatch()->glFrustumx(left, right, bottom, top, zNear, zFar);
}

void glGetBooleanv(GLenum pname, GLboolean *params)
{
     getDispatch()->glGetBooleanv(pname, params);
}

void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
     getDispatch()->glGetBufferParameteriv(target, pname, params);
}

void glGetClipPlanex(GLenum pname, GLfixed eqn[4])
{
     getDispatch()->glGetClipPlanex(pname, eqn);
}

void glGenBuffers(GLsizei n, GLuint *buffers)
{
     getDispatch()->glGenBuffers(n, buffers);
}

void glGenTextures(GLsizei n, GLuint *textures)
{
     getDispatch()->glGenTextures(n, textures);
}

GLenum glGetError()
{
     return getDispatch()->glGetError();
}

void glGetFixedv(GLenum pname, GLfixed *params)
{
     getDispatch()->glGetFixedv(pname, params);
}

void glGetIntegerv(GLenum pname, GLint *params)
{
     getDispatch()->glGetIntegerv(pname, params);
}

void glGetLightxv(GLenum light, GLenum pname, GLfixed *params)
{
     getDispatch()->glGetLightxv(light, pname, params);
}

void glGetMaterialxv(GLenum face, GLenum pname, GLfixed *params)
{
     getDispatch()->glGetMaterialxv(face, pname, params);
}

void glGetPointerv(GLenum pname, GLvoid **params)
{
     getDispatch()->glGetPointerv(pname, params);
}

const GLubyte* glGetString(GLenum name)
{
     return getDispatch()->glGetString(name);
}

void glGetTexEnviv(GLenum env, GLenum pname, GLint *params)
{
     getDispatch()->glGetTexEnviv(env, pname, params);
}

void glGetTexEnvxv(GLenum env, GLenum pname, GLfixed *params)
{
     getDispatch()->glGetTexEnvxv(env, pname, params);
}

void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
     getDispatch()->glGetTexParameteriv(target, pname, params);
}

void glGetTexParameterxv(GLenum target, GLenum pname, GLfixed *params)
{
     getDispatch()->glGetTexParameterxv(target, pname, params);
}

void glHint(GLenum target, GLenum mode)
{
     getDispatch()->glHint(target, mode);
}

GLboolean glIsBuffer(GLuint buffer)
{
     return getDispatch()->glIsBuffer(buffer);
}

GLboolean glIsEnabled(GLenum cap)
{
     return getDispatch()->glIsEnabled(cap);
}

GLboolean glIsTexture(GLuint texture)
{
     return getDispatch()->glIsTexture(texture);
}

void glLightModelx(GLenum pname, GLfixed param)
{
     getDispatch()->glLightModelx(pname, param);
}

void glLightModelxv(GLenum pname, const GLfixed *params)
{
     getDispatch()->glLightModelxv(pname, params);
}

void glLightx(GLenum light, GLenum pname, GLfixed param)
{
     getDispatch()->glLightx(light, pname, param);
}

void glLightxv(GLenum light, GLenum pname, const GLfixed *params)
{
     getDispatch()->glLightxv(light, pname, params);
}

void glLineWidthx(GLfixed width)
{
     getDispatch()->glLineWidthx(width);
}

void glLoadIdentity()
{
     getDispatch()->glLoadIdentity();
}

void glLoadMatrixx(const GLfixed *m)
{
     getDispatch()->glLoadMatrixx(m);
}

void glLogicOp(GLenum opcode)
{
     getDispatch()->glLogicOp(opcode);
}

void glMaterialx(GLenum face, GLenum pname, GLfixed param)
{
     getDispatch()->glMaterialx(face, pname, param);
}

void glMaterialxv(GLenum face, GLenum pname, const GLfixed *params)
{
     getDispatch()->glMaterialxv(face, pname, params);
}

void glMatrixMode(GLenum mode)
{
     getDispatch()->glMatrixMode(mode);
}

void glMultMatrixx(const GLfixed *m)
{
     getDispatch()->glMultMatrixx(m);
}

void glMultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
     getDispatch()->glMultiTexCoord4x(target, s, t, r, q);
}

void glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz)
{
     getDispatch()->glNormal3x(nx, ny, nz);
}

void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
     getDispatch()->glNormalPointer(type, stride, pointer);
}

void glOrthox(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
     getDispatch()->glOrthox(left, right, bottom, top, zNear, zFar);
}

void glPixelStorei(GLenum pname, GLint param)
{
     getDispatch()->glPixelStorei(pname, param);
}

void glPointParameterx(GLenum pname, GLfixed param)
{
     getDispatch()->glPointParameterx(pname, param);
}

void glPointParameterxv(GLenum pname, const GLfixed *params)
{
     getDispatch()->glPointParameterxv(pname, params);
}

void glPointSizex(GLfixed size)
{
     getDispatch()->glPointSizex(size);
}

void glPolygonOffsetx(GLfixed factor, GLfixed units)
{
     getDispatch()->glPolygonOffsetx(factor, units);
}

void glPopMatrix()
{
     getDispatch()->glPopMatrix();
}

void glPushMatrix()
{
     getDispatch()->glPushMatrix();
}

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
     getDispatch()->glReadPixels(x, y, width, height, format, type, pixels);
}

void glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
     getDispatch()->glRotatex(angle, x, y, z);
}

void glSampleCoverage(GLclampf value, GLboolean invert)
{
     getDispatch()->glSampleCoverage(value, invert);
}

void glSampleCoveragex(GLclampx value, GLboolean invert)
{
     getDispatch()->glSampleCoveragex(value, invert);
}

void glScalex(GLfixed x, GLfixed y, GLfixed z)
{
     getDispatch()->glScalex(x, y, z);
}

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
     getDispatch()->glScissor(x, y, width, height);
}

void glShadeModel(GLenum mode)
{
     getDispatch()->glShadeModel(mode);
}

void glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
     getDispatch()->glStencilFunc(func, ref, mask);
}

void glStencilMask(GLuint mask)
{
     getDispatch()->glStencilMask(mask);
}

void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
     getDispatch()->glStencilOp(fail, zfail, zpass);
}

void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
     getDispatch()->glTexCoordPointer(size, type, stride, pointer);
}

void glTexEnvi(GLenum target, GLenum pname, GLint param)
{
     getDispatch()->glTexEnvi(target, pname, param);
}

void glTexEnvx(GLenum target, GLenum pname, GLfixed param)
{
     getDispatch()->glTexEnvx(target, pname, param);
}

void glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
     getDispatch()->glTexEnviv(target, pname, params);
}

void glTexEnvxv(GLenum target, GLenum pname, const GLfixed *params)
{
     getDispatch()->glTexEnvxv(target, pname, params);
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
     getDispatch()->glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
     getDispatch()->glTexParameteri(target, pname, param);
}

void glTexParameterx(GLenum target, GLenum pname, GLfixed param)
{
     getDispatch()->glTexParameterx(target, pname, param);
}

void glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
     getDispatch()->glTexParameteriv(target, pname, params);
}

void glTexParameterxv(GLenum target, GLenum pname, const GLfixed *params)
{
     getDispatch()->glTexParameterxv(target, pname, params);
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
     getDispatch()->glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void glTranslatex(GLfixed x, GLfixed y, GLfixed z)
{
     getDispatch()->glTranslatex(x, y, z);
}

void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
     getDispatch()->glVertexPointer(size, type, stride, pointer);
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
     getDispatch()->glViewport(x, y, width, height);
}

void glPointSizePointerOES(GLenum type, GLsizei stride, const GLvoid *pointer)
{
     getDispatch()->glPointSizePointerOES(type, stride, pointer);
}

void glBlendEquationSeparateOES(GLenum modeRGB, GLenum modeAlpha)
{
     getDispatch()->glBlendEquationSeparateOES(modeRGB, modeAlpha);
}

void glBlendFuncSeparateOES(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
     getDispatch()->glBlendFuncSeparateOES(srcRGB, dstRGB, srcAlpha, dstAlpha);
}

void glBlendEquationOES(GLenum mode)
{
     getDispatch()->glBlendEquationOES(mode);
}

void glDrawTexsOES(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height)
{
     getDispatch()->glDrawTexsOES(x, y, z, width, height);
}

void glDrawTexiOES(GLint x, GLint y, GLint z, GLint width, GLint height)
{
     getDispatch()->glDrawTexiOES(x, y, z, width, height);
}

void glDrawTexxOES(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height)
{
     getDispatch()->glDrawTexxOES(x, y, z, width, height);
}

void glDrawTexsvOES(const GLshort *coords)
{
     getDispatch()->glDrawTexsvOES(coords);
}

void glDrawTexivOES(const GLint *coords)
{
     getDispatch()->glDrawTexivOES(coords);
}

void glDrawTexxvOES(const GLfixed *coords)
{
     getDispatch()->glDrawTexxvOES(coords);
}

void glDrawTexfOES(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height)
{
     getDispatch()->glDrawTexfOES(x, y, z, width, height);
}

void glDrawTexfvOES(const GLfloat *coords)
{
     getDispatch()->glDrawTexfvOES(coords);
}

void glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
     getDispatch()->glEGLImageTargetTexture2DOES(target, image);
}

void glEGLImageTargetRenderbufferStorageOES(GLenum target, GLeglImageOES image)
{
     getDispatch()->glEGLImageTargetRenderbufferStorageOES(target, image);
}

void glAlphaFuncxOES(GLenum func, GLclampx ref)
{
     getDispatch()->glAlphaFuncxOES(func, ref);
}

void glClearColorxOES(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha)
{
     getDispatch()->glClearColorxOES(red, green, blue, alpha);
}

void glClearDepthxOES(GLclampx depth)
{
     getDispatch()->glClearDepthxOES(depth);
}

void glClipPlanexOES(GLenum plane, const GLfixed *equation)
{
     getDispatch()->glClipPlanexOES(plane, equation);
}

void glColor4xOES(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
     getDispatch()->glColor4xOES(red, green, blue, alpha);
}

void glDepthRangexOES(GLclampx zNear, GLclampx zFar)
{
     getDispatch()->glDepthRangexOES(zNear, zFar);
}

void glFogxOES(GLenum pname, GLfixed param)
{
     getDispatch()->glFogxOES(pname, param);
}

void glFogxvOES(GLenum pname, const GLfixed *params)
{
     getDispatch()->glFogxvOES(pname, params);
}

void glFrustumxOES(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
     getDispatch()->glFrustumxOES(left, right, bottom, top, zNear, zFar);
}

void glGetClipPlanexOES(GLenum pname, GLfixed eqn[4])
{
     getDispatch()->glGetClipPlanexOES(pname, eqn);
}

void glGetFixedvOES(GLenum pname, GLfixed *params)
{
     getDispatch()->glGetFixedvOES(pname, params);
}

void glGetLightxvOES(GLenum light, GLenum pname, GLfixed *params)
{
     getDispatch()->glGetLightxvOES(light, pname, params);
}

void glGetMaterialxvOES(GLenum face, GLenum pname, GLfixed *params)
{
     getDispatch()->glGetMaterialxvOES(face, pname, params);
}

void glGetTexEnvxvOES(GLenum env, GLenum pname, GLfixed *params)
{
     getDispatch()->glGetTexEnvxvOES(env, pname, params);
}

void glGetTexParameterxvOES(GLenum target, GLenum pname, GLfixed *params)
{
     getDispatch()->glGetTexParameterxvOES(target, pname, params);
}

void glLightModelxOES(GLenum pname, GLfixed param)
{
     getDispatch()->glLightModelxOES(pname, param);
}

void glLightModelxvOES(GLenum pname, const GLfixed *params)
{
     getDispatch()->glLightModelxvOES(pname, params);
}

void glLightxOES(GLenum light, GLenum pname, GLfixed param)
{
     getDispatch()->glLightxOES(light, pname, param);
}

void glLightxvOES(GLenum light, GLenum pname, const GLfixed *params)
{
     getDispatch()->glLightxvOES(light, pname, params);
}

void glLineWidthxOES(GLfixed width)
{
     getDispatch()->glLineWidthxOES(width);
}

void glLoadMatrixxOES(const GLfixed *m)
{
     getDispatch()->glLoadMatrixxOES(m);
}

void glMaterialxOES(GLenum face, GLenum pname, GLfixed param)
{
     getDispatch()->glMaterialxOES(face, pname, param);
}

void glMaterialxvOES(GLenum face, GLenum pname, const GLfixed *params)
{
     getDispatch()->glMaterialxvOES(face, pname, params);
}

void glMultMatrixxOES(const GLfixed *m)
{
     getDispatch()->glMultMatrixxOES(m);
}

void glMultiTexCoord4xOES(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
     getDispatch()->glMultiTexCoord4xOES(target, s, t, r, q);
}

void glNormal3xOES(GLfixed nx, GLfixed ny, GLfixed nz)
{
     getDispatch()->glNormal3xOES(nx, ny, nz);
}

void glOrthoxOES(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
     getDispatch()->glOrthoxOES(left, right, bottom, top, zNear, zFar);
}

void glPointParameterxOES(GLenum pname, GLfixed param)
{
     getDispatch()->glPointParameterxOES(pname, param);
}

void glPointParameterxvOES(GLenum pname, const GLfixed *params)
{
     getDispatch()->glPointParameterxvOES(pname, params);
}

void glPointSizexOES(GLfixed size)
{
     getDispatch()->glPointSizexOES(size);
}

void glPolygonOffsetxOES(GLfixed factor, GLfixed units)
{
     getDispatch()->glPolygonOffsetxOES(factor, units);
}

void glRotatexOES(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
     getDispatch()->glRotatexOES(angle, x, y, z);
}

void glSampleCoveragexOES(GLclampx value, GLboolean invert)
{
     getDispatch()->glSampleCoveragexOES(value, invert);
}

void glScalexOES(GLfixed x, GLfixed y, GLfixed z)
{
     getDispatch()->glScalexOES(x, y, z);
}

void glTexEnvxOES(GLenum target, GLenum pname, GLfixed param)
{
     getDispatch()->glTexEnvxOES(target, pname, param);
}

void glTexEnvxvOES(GLenum target, GLenum pname, const GLfixed *params)
{
     getDispatch()->glTexEnvxvOES(target, pname, params);
}

void glTexParameterxOES(GLenum target, GLenum pname, GLfixed param)
{
     getDispatch()->glTexParameterxOES(target, pname, param);
}

void glTexParameterxvOES(GLenum target, GLenum pname, const GLfixed *params)
{
     getDispatch()->glTexParameterxvOES(target, pname, params);
}

void glTranslatexOES(GLfixed x, GLfixed y, GLfixed z)
{
     getDispatch()->glTranslatexOES(x, y, z);
}

GLboolean glIsRenderbufferOES(GLuint renderbuffer)
{
     return getDispatch()->glIsRenderbufferOES(renderbuffer);
}

void glBindRenderbufferOES(GLenum target, GLuint renderbuffer)
{
     getDispatch()->glBindRenderbufferOES(target, renderbuffer);
}

void glDeleteRenderbuffersOES(GLsizei n, const GLuint *renderbuffers)
{
     getDispatch()->glDeleteRenderbuffersOES(n, renderbuffers);
}

void glGenRenderbuffersOES(GLsizei n, GLuint *renderbuffers)
{
     getDispatch()->glGenRenderbuffersOES(n, renderbuffers);
}

void glRenderbufferStorageOES(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
     getDispatch()->glRenderbufferStorageOES(target, internalformat, width, height);
}

void glGetRenderbufferParameterivOES(GLenum target, GLenum pname, GLint *params)
{
     getDispatch()->glGetRenderbufferParameterivOES(target, pname, params);
}

GLboolean glIsFramebufferOES(GLuint framebuffer)
{
     return getDispatch()->glIsFramebufferOES(framebuffer);
}

void glBindFramebufferOES(GLenum target, GLuint framebuffer)
{
     getDispatch()->glBindFramebufferOES(target, framebuffer);
}

void glDeleteFramebuffersOES(GLsizei n, const GLuint *framebuffers)
{
     getDispatch()->glDeleteFramebuffersOES(n, framebuffers);
}

void glGenFramebuffersOES(GLsizei n, GLuint *framebuffers)
{
     getDispatch()->glGenFramebuffersOES(n, framebuffers);
}

GLenum glCheckFramebufferStatusOES(GLenum target)
{
     return getDispatch()->glCheckFramebufferStatusOES(target);
}

void glFramebufferRenderbufferOES(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
     getDispatch()->glFramebufferRenderbufferOES(target, attachment, renderbuffertarget, renderbuffer);
}

void glFramebufferTexture2DOES(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
     getDispatch()->glFramebufferTexture2DOES(target, attachment, textarget, texture, level);
}

void glGetFramebufferAttachmentParameterivOES(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
     getDispatch()->glGetFramebufferAttachmentParameterivOES(target, attachment, pname, params);
}

void glGenerateMipmapOES(GLenum target)
{
     getDispatch()->glGenerateMipmapOES(target);
}

void* glMapBufferOES(GLenum target, GLenum access)
{
     return getDispatch()->glMapBufferOES(target, access);
}

GLboolean glUnmapBufferOES(GLenum target)
{
     return getDispatch()->glUnmapBufferOES(target);
}

void glGetBufferPointervOES(GLenum target, GLenum pname, GLvoid **ptr)
{
     getDispatch()->glGetBufferPointervOES(target, pname, ptr);
}

void glCurrentPaletteMatrixOES(GLuint matrixpaletteindex)
{
     getDispatch()->glCurrentPaletteMatrixOES(matrixpaletteindex);
}

void glLoadPaletteFromModelViewMatrixOES()
{
     getDispatch()->glLoadPaletteFromModelViewMatrixOES();
}

void glMatrixIndexPointerOES(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
     getDispatch()->glMatrixIndexPointerOES(size, type, stride, pointer);
}

void glWeightPointerOES(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
     getDispatch()->glWeightPointerOES(size, type, stride, pointer);
}

GLbitfield glQueryMatrixxOES(GLfixed mantissa[16], GLint exponent[16])
{
     return getDispatch()->glQueryMatrixxOES(mantissa, exponent);
}

void glDepthRangefOES(GLclampf zNear, GLclampf zFar)
{
     getDispatch()->glDepthRangefOES(zNear, zFar);
}

void glFrustumfOES(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
     getDispatch()->glFrustumfOES(left, right, bottom, top, zNear, zFar);
}

void glOrthofOES(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
     getDispatch()->glOrthofOES(left, right, bottom, top, zNear, zFar);
}

void glClipPlanefOES(GLenum plane, const GLfloat *equation)
{
     getDispatch()->glClipPlanefOES(plane, equation);
}

void glGetClipPlanefOES(GLenum pname, GLfloat eqn[4])
{
     getDispatch()->glGetClipPlanefOES(pname, eqn);
}

void glClearDepthfOES(GLclampf depth)
{
     getDispatch()->glClearDepthfOES(depth);
}

void glTexGenfOES(GLenum coord, GLenum pname, GLfloat param)
{
     getDispatch()->glTexGenfOES(coord, pname, param);
}

void glTexGenfvOES(GLenum coord, GLenum pname, const GLfloat *params)
{
     getDispatch()->glTexGenfvOES(coord, pname, params);
}

void glTexGeniOES(GLenum coord, GLenum pname, GLint param)
{
     getDispatch()->glTexGeniOES(coord, pname, param);
}

void glTexGenivOES(GLenum coord, GLenum pname, const GLint *params)
{
     getDispatch()->glTexGenivOES(coord, pname, params);
}

void glTexGenxOES(GLenum coord, GLenum pname, GLfixed param)
{
     getDispatch()->glTexGenxOES(coord, pname, param);
}

void glTexGenxvOES(GLenum coord, GLenum pname, const GLfixed *params)
{
     getDispatch()->glTexGenxvOES(coord, pname, params);
}

void glGetTexGenfvOES(GLenum coord, GLenum pname, GLfloat *params)
{
     getDispatch()->glGetTexGenfvOES(coord, pname, params);
}

void glGetTexGenivOES(GLenum coord, GLenum pname, GLint *params)
{
     getDispatch()->glGetTexGenivOES(coord, pname, params);
}

void glGetTexGenxvOES(GLenum coord, GLenum pname, GLfixed *params)
{
     getDispatch()->glGetTexGenxvOES(coord, pname, params);
}

void glBindVertexArrayOES(GLuint array)
{
     getDispatch()->glBindVertexArrayOES(array);
}

void glDeleteVertexArraysOES(GLsizei n, const GLuint *arrays)
{
     getDispatch()->glDeleteVertexArraysOES(n, arrays);
}

void glGenVertexArraysOES(GLsizei n, GLuint *arrays)
{
     getDispatch()->glGenVertexArraysOES(n, arrays);
}

GLboolean glIsVertexArrayOES(GLuint array)
{
     return getDispatch()->glIsVertexArrayOES(array);
}

void glDiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
     getDispatch()->glDiscardFramebufferEXT(target, numAttachments, attachments);
}

void glMultiDrawArraysEXT(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount)
{
     getDispatch()->glMultiDrawArraysEXT(mode, first, count, primcount);
}

void glMultiDrawElementsEXT(GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount)
{
     getDispatch()->glMultiDrawElementsEXT(mode, count, type, indices, primcount);
}

void glClipPlanefIMG(GLenum p, const GLfloat *eqn)
{
     getDispatch()->glClipPlanefIMG(p, eqn);
}

void glClipPlanexIMG(GLenum p, const GLfixed *eqn)
{
     getDispatch()->glClipPlanexIMG(p, eqn);
}

void glRenderbufferStorageMultisampleIMG(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
     getDispatch()->glRenderbufferStorageMultisampleIMG(target, samples, internalformat, width, height);
}

void glFramebufferTexture2DMultisampleIMG(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
     getDispatch()->glFramebufferTexture2DMultisampleIMG(target, attachment, textarget, texture, level, samples);
}

void glDeleteFencesNV(GLsizei n, const GLuint *fences)
{
     getDispatch()->glDeleteFencesNV(n, fences);
}

void glGenFencesNV(GLsizei n, GLuint *fences)
{
     getDispatch()->glGenFencesNV(n, fences);
}

GLboolean glIsFenceNV(GLuint fence)
{
     return getDispatch()->glIsFenceNV(fence);
}

GLboolean glTestFenceNV(GLuint fence)
{
     return getDispatch()->glTestFenceNV(fence);
}

void glGetFenceivNV(GLuint fence, GLenum pname, GLint *params)
{
     getDispatch()->glGetFenceivNV(fence, pname, params);
}

void glFinishFenceNV(GLuint fence)
{
     getDispatch()->glFinishFenceNV(fence);
}

void glSetFenceNV(GLuint fence, GLenum condition)
{
     getDispatch()->glSetFenceNV(fence, condition);
}

void glGetDriverControlsQCOM(GLint *num, GLsizei size, GLuint *driverControls)
{
     getDispatch()->glGetDriverControlsQCOM(num, size, driverControls);
}

void glGetDriverControlStringQCOM(GLuint driverControl, GLsizei bufSize, GLsizei *length, GLchar *driverControlString)
{
     getDispatch()->glGetDriverControlStringQCOM(driverControl, bufSize, length, driverControlString);
}

void glEnableDriverControlQCOM(GLuint driverControl)
{
     getDispatch()->glEnableDriverControlQCOM(driverControl);
}

void glDisableDriverControlQCOM(GLuint driverControl)
{
     getDispatch()->glDisableDriverControlQCOM(driverControl);
}

void glExtGetTexturesQCOM(GLuint *textures, GLint maxTextures, GLint *numTextures)
{
     getDispatch()->glExtGetTexturesQCOM(textures, maxTextures, numTextures);
}

void glExtGetBuffersQCOM(GLuint *buffers, GLint maxBuffers, GLint *numBuffers)
{
     getDispatch()->glExtGetBuffersQCOM(buffers, maxBuffers, numBuffers);
}

void glExtGetRenderbuffersQCOM(GLuint *renderbuffers, GLint maxRenderbuffers, GLint *numRenderbuffers)
{
     getDispatch()->glExtGetRenderbuffersQCOM(renderbuffers, maxRenderbuffers, numRenderbuffers);
}

void glExtGetFramebuffersQCOM(GLuint *framebuffers, GLint maxFramebuffers, GLint *numFramebuffers)
{
     getDispatch()->glExtGetFramebuffersQCOM(framebuffers, maxFramebuffers, numFramebuffers);
}

void glExtGetTexLevelParameterivQCOM(GLuint texture, GLenum face, GLint level, GLenum pname, GLint *params)
{
     getDispatch()->glExtGetTexLevelParameterivQCOM(texture, face, level, pname, params);
}

void glExtTexObjectStateOverrideiQCOM(GLenum target, GLenum pname, GLint param)
{
     getDispatch()->glExtTexObjectStateOverrideiQCOM(target, pname, param);
}

void glExtGetTexSubImageQCOM(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLvoid *texels)
{
     getDispatch()->glExtGetTexSubImageQCOM(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, texels);
}

void glExtGetBufferPointervQCOM(GLenum target, GLvoid **params)
{
     getDispatch()->glExtGetBufferPointervQCOM(target, params);
}

void glExtGetShadersQCOM(GLuint *shaders, GLint maxShaders, GLint *numShaders)
{
     getDispatch()->glExtGetShadersQCOM(shaders, maxShaders, numShaders);
}

void glExtGetProgramsQCOM(GLuint *programs, GLint maxPrograms, GLint *numPrograms)
{
     getDispatch()->glExtGetProgramsQCOM(programs, maxPrograms, numPrograms);
}

GLboolean glExtIsProgramBinaryQCOM(GLuint program)
{
     return getDispatch()->glExtIsProgramBinaryQCOM(program);
}

void glExtGetProgramBinarySourceQCOM(GLuint program, GLenum shadertype, GLchar *source, GLint *length)
{
     getDispatch()->glExtGetProgramBinarySourceQCOM(program, shadertype, source, length);
}

void glStartTilingQCOM(GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask)
{
     getDispatch()->glStartTilingQCOM(x, y, width, height, preserveMask);
}

void glEndTilingQCOM(GLbitfield preserveMask)
{
     getDispatch()->glEndTilingQCOM(preserveMask);
}

