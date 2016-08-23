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
#ifndef _TEXTURE_UTILS_H
#define _TEXTURE_UTILS_H

#include <GLES/gl.h>
#include <GLES/glext.h>
#include "GLEScontext.h"
#include "PaletteTexture.h"
#include "etc1.h"

int getCompressedFormats(int* formats);
void  doCompressedTexImage2D(GLEScontext * ctx, GLenum target, GLint level, 
                                          GLenum internalformat, GLsizei width, 
                                          GLsizei height, GLint border, 
                                          GLsizei imageSize, const GLvoid* data, void * funcPtr);

#endif
