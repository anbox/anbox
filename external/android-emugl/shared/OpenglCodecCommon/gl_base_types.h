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
#ifndef __GL_BASE_TYPES__H
#define __GL_BASE_TYPES__H

#include <KHR/khrplatform.h>

#ifndef gles1_APIENTRY
#define gles1_APIENTRY KHRONOS_APIENTRY
#endif

#ifndef gles2_APIENTRY
#define gles2_APIENTRY KHRONOS_APIENTRY
#endif

typedef void             GLvoid;
typedef unsigned int     GLenum;
typedef unsigned char    GLboolean;
typedef unsigned int     GLbitfield;
typedef char             GLchar;
typedef khronos_int8_t   GLbyte;
typedef short            GLshort;
typedef int              GLint;
typedef int              GLsizei;
typedef khronos_uint8_t  GLubyte;
typedef unsigned short   GLushort;
typedef unsigned int     GLuint;
typedef khronos_float_t  GLfloat;
typedef khronos_float_t  GLclampf;
typedef khronos_int32_t  GLfixed;
typedef khronos_int32_t  GLclampx;
typedef khronos_intptr_t GLintptr;
typedef khronos_ssize_t  GLsizeiptr;
typedef char *GLstr;
/* JR XXX Treating this as an in handle - is this correct? */
typedef void * GLeglImageOES;

/* ErrorCode */
#ifndef GL_INVALID_ENUM
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_OUT_OF_MEMORY                  0x0505
#endif

#endif
