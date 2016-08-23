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

#ifndef GLES_V2_VALIDATE_H
#define GLES_V2_VALIDATE_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLcommon/GLESvalidate.h>

struct GLESv2Validate:public GLESvalidate{
static bool blendEquationMode(GLenum mode);
static bool blendSrc(GLenum s);
static bool blendDst(GLenum d);
static bool textureParams(GLenum param);
static bool hintTargetMode(GLenum target,GLenum mode);
static bool capability(GLenum cap);
static bool pixelStoreParam(GLenum param);
static bool readPixelFrmt(GLenum format);
static bool shaderType(GLenum type);
static bool precisionType(GLenum type);
static bool arrayIndex(GLEScontext * ctx,GLuint index);
static bool pixelType(GLEScontext * ctx,GLenum type);
static bool pixelFrmt(GLEScontext* ctx,GLenum format);
static bool attribName(const GLchar* name);
static bool attribIndex(int index);
static bool programParam(GLenum pname);
static bool textureIsCubeMap(GLenum target);
};

#endif
