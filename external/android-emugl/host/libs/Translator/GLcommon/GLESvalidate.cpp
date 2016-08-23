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

#include <GLcommon/GLESvalidate.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <OpenglCodecCommon/ErrorLog.h>


bool  GLESvalidate::textureEnum(GLenum e,unsigned int maxTex) {
    return e >= GL_TEXTURE0 && e < (GL_TEXTURE0 + maxTex);
}

bool GLESvalidate::pixelType(GLEScontext * ctx, GLenum type) {
    if ((ctx && ctx->getCaps()->GL_EXT_PACKED_DEPTH_STENCIL) &&
       (type == GL_UNSIGNED_INT_24_8_OES) )
        return true;

    if (ctx &&
       (ctx->getCaps()->GL_ARB_HALF_FLOAT_PIXEL || ctx->getCaps()->GL_NV_HALF_FLOAT) &&
       (type == GL_HALF_FLOAT_OES))
        return true;

    switch(type) {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_FLOAT:
        return true;
    }
    return false;
}

bool GLESvalidate::pixelOp(GLenum format,GLenum type) {
     switch(type) {
     case GL_UNSIGNED_SHORT_4_4_4_4:
     case GL_UNSIGNED_SHORT_5_5_5_1:
         return format == GL_RGBA;
     case GL_UNSIGNED_SHORT_5_6_5:
         return format == GL_RGB;
     }
     return true;
}

bool GLESvalidate::pixelFrmt(GLEScontext* ctx ,GLenum format) {
    if (ctx && ctx->getCaps()->GL_EXT_TEXTURE_FORMAT_BGRA8888 && format == GL_BGRA_EXT)
      return true;
    if (ctx && ctx->getCaps()->GL_EXT_PACKED_DEPTH_STENCIL && format == GL_DEPTH_STENCIL_OES)
      return true;
    switch(format) {
    case GL_ALPHA:
    case GL_RGB:
    case GL_RGBA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
        return true;
    }
    return false;
}

bool GLESvalidate::bufferTarget(GLenum target) {
    return target == GL_ARRAY_BUFFER || target == GL_ELEMENT_ARRAY_BUFFER;
}

bool GLESvalidate::bufferUsage(GLenum usage) {
    switch(usage) {
        case GL_STREAM_DRAW:
        case GL_STATIC_DRAW:
        case GL_DYNAMIC_DRAW:
            return true;
    }
    return false;
}

bool GLESvalidate::bufferParam(GLenum param) {
 return  (param == GL_BUFFER_SIZE) || (param == GL_BUFFER_USAGE);
}

bool GLESvalidate::drawMode(GLenum mode) {
    switch(mode) {
    case GL_POINTS:
    case GL_LINE_STRIP:
    case GL_LINE_LOOP:
    case GL_LINES:
    case GL_TRIANGLE_STRIP:
    case GL_TRIANGLE_FAN:
    case GL_TRIANGLES:
        return true;
    }
    return false;
}

bool GLESvalidate::drawType(GLenum mode) {
    return  mode == GL_UNSIGNED_BYTE ||
            mode == GL_UNSIGNED_SHORT ||
            mode == GL_UNSIGNED_INT;
}

bool GLESvalidate::textureTarget(GLenum target) {
    return target==GL_TEXTURE_2D || target==GL_TEXTURE_CUBE_MAP;
}

bool GLESvalidate::textureTargetLimited(GLenum target) {
    return target==GL_TEXTURE_2D;
}

bool GLESvalidate::textureTargetEx(GLenum target) {
    switch(target) {
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES:
    case GL_TEXTURE_2D:
      return true;
    } 
    return false;
}

bool GLESvalidate::blendEquationMode(GLenum mode){
    return mode == GL_FUNC_ADD             ||
           mode == GL_FUNC_SUBTRACT        ||
           mode == GL_FUNC_REVERSE_SUBTRACT;
}

bool GLESvalidate::framebufferTarget(GLenum target){
    return target == GL_FRAMEBUFFER;
}

bool GLESvalidate::framebufferAttachment(GLenum attachment){
    switch(attachment){
    case GL_COLOR_ATTACHMENT0:
    case GL_DEPTH_ATTACHMENT:
    case GL_STENCIL_ATTACHMENT:
        return true;
    }
    return false;
}

bool GLESvalidate::framebufferAttachmentParams(GLenum pname){
    switch(pname){
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
        return true;
    }
    return false;
}

bool GLESvalidate::renderbufferTarget(GLenum target){
    return target == GL_RENDERBUFFER;
}

bool GLESvalidate::renderbufferParams(GLenum pname){
    switch(pname){
    case GL_RENDERBUFFER_WIDTH:
    case GL_RENDERBUFFER_HEIGHT:
    case GL_RENDERBUFFER_INTERNAL_FORMAT:
    case GL_RENDERBUFFER_RED_SIZE:
    case GL_RENDERBUFFER_GREEN_SIZE:
    case GL_RENDERBUFFER_BLUE_SIZE:
    case GL_RENDERBUFFER_ALPHA_SIZE:
    case GL_RENDERBUFFER_DEPTH_SIZE:
    case GL_RENDERBUFFER_STENCIL_SIZE:
        return true;
    }
    return false;
}

bool GLESvalidate::texImgDim(GLsizei width,GLsizei height,int maxTexSize) {

 if( width < 0 || height < 0 || width > maxTexSize || height > maxTexSize)
    return false;
 return isPowerOf2(width) && isPowerOf2(height);
}

