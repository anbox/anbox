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
#ifndef __GL_UTILS_H__
#define __GL_UTILS_H__

#include <stdio.h>
#include <stdlib.h>

#ifdef GL_API
    #undef GL_API
#endif
#define GL_API

#ifdef GL_APIENTRY
    #undef GL_APIENTRY
#endif

#ifdef GL_APIENTRYP
    #undef GL_APIENTRYP
#endif
#define GL_APIENTRYP

#ifndef ANDROID
#define GL_APIENTRY
#endif

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifdef __cplusplus
extern "C" {
#endif

    size_t glSizeof(GLenum type);
    size_t glUtilsParamSize(GLenum param);
    void   glUtilsPackPointerData(unsigned char *dst, unsigned char *str,
                           int size, GLenum type, unsigned int stride,
                           unsigned int datalen);
    void glUtilsWritePackPointerData(void* stream, unsigned char *src,
                                    int size, GLenum type, unsigned int stride,
                                    unsigned int datalen);
    int glUtilsPixelBitSize(GLenum format, GLenum type);
    void   glUtilsPackStrings(char *ptr, char **strings, GLint *length, GLsizei count);
    int glUtilsCalcShaderSourceLen(char **strings, GLint *length, GLsizei count);
#ifdef __cplusplus
};
#endif

namespace GLUtils {

    template <class T> void minmax(T *indices, int count, int *min, int *max) {
        *min = -1;
        *max = -1;
        T *ptr = indices;
        for (int i = 0; i < count; i++) {
            if (*min == -1 || *ptr < *min) *min = *ptr;
            if (*max == -1 || *ptr > *max) *max = *ptr;
            ptr++;
        }
    }

    template <class T> void shiftIndices(T *indices, int count,  int offset) {
        T *ptr = indices;
        for (int i = 0; i < count; i++) {
            *ptr += offset;
            ptr++;
        }
    }


    template <class T> void shiftIndices(T *src, T *dst, int count, int offset)
    {
        for (int i = 0; i < count; i++) {
            *dst = *src + offset;
            dst++;
            src++;
        }
    }
}; // namespace GLUtils
#endif
