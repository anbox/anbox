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
#ifndef GLES_VALIDATE_H
#define GLES_VALIDATE_H

#include <GLES/gl.h>
#include "GLEScontext.h"
struct GLESvalidate
{
static bool textureEnum(GLenum e,unsigned int maxTex);
static bool pixelType(GLEScontext * ctx,GLenum type);
static bool pixelOp(GLenum format,GLenum type);
static bool pixelFrmt(GLEScontext* ctx , GLenum format);
static bool bufferTarget(GLenum target);
static bool bufferUsage(GLenum usage);
static bool bufferParam(GLenum param);
static bool drawMode(GLenum mode);
static bool drawType(GLenum mode);
static bool textureTarget(GLenum target);
static bool textureTargetLimited(GLenum target);
static bool textureTargetEx(GLenum target);
static bool texImgDim(GLsizei width,GLsizei height,int maxTexSize);
static bool blendEquationMode(GLenum mode);
static bool framebufferTarget(GLenum target);
static bool framebufferAttachment(GLenum attachment);
static bool framebufferAttachmentParams(GLenum pname);
static bool renderbufferTarget(GLenum target);
static bool renderbufferParams(GLenum pname);
};

#endif
