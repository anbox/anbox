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
#include "GLEScmValidate.h"
#include <GLcommon/GLutils.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLcommon/GLEScontext.h>
#include "GLEScmValidate.h"


bool GLEScmValidate::lightEnum(GLenum e,unsigned int maxLights) {
    return  e >=GL_LIGHT0 && e <= (GL_LIGHT0+maxLights);
}

bool GLEScmValidate::clipPlaneEnum(GLenum e,unsigned int maxClipPlanes) {
    return  e >=GL_CLIP_PLANE0 && e <= (GL_CLIP_PLANE0+maxClipPlanes);
}

bool GLEScmValidate::alphaFunc(GLenum f) {
    switch(f) {
    case GL_NEVER:
    case GL_LESS:
    case GL_EQUAL:
    case GL_LEQUAL:
    case GL_GREATER:
    case GL_NOTEQUAL:
    case GL_GEQUAL:
    case GL_ALWAYS:
        return true;
    }
    return false;
}

bool GLEScmValidate::blendSrc(GLenum s) {
   switch(s) {
    case GL_ZERO:
    case GL_ONE:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
        return true;
  }
  return false;
}

bool GLEScmValidate::vertexPointerParams(GLint size,GLsizei stride) {
    return ((size >=2) && (size <= 4)) && (stride >=0) ;
}

bool GLEScmValidate::colorPointerParams(GLint size,GLsizei stride) {
    return (size == 4) && (stride >=0) ;
}

bool GLEScmValidate::texCoordPointerParams(GLint size,GLsizei stride) {
    return ((size >=2) && (size <= 4)) && (stride >=0) ;
}

bool GLEScmValidate::supportedArrays(GLenum arr) {
    switch(arr) {
    case GL_COLOR_ARRAY:
    case GL_NORMAL_ARRAY:
    case GL_POINT_SIZE_ARRAY_OES:
    case GL_TEXTURE_COORD_ARRAY:
    case GL_VERTEX_ARRAY:
        return true;
    }
    return false;
}

bool GLEScmValidate::hintTargetMode(GLenum target,GLenum mode) {
   switch(target) {
   case GL_FOG_HINT:
   case GL_GENERATE_MIPMAP_HINT:
   case GL_LINE_SMOOTH_HINT:
   case GL_PERSPECTIVE_CORRECTION_HINT:
   case GL_POINT_SMOOTH_HINT:
       break;
   default: return false;
   }
   switch(mode) {
   case GL_FASTEST:
   case GL_NICEST:
   case GL_DONT_CARE:
       break;
   default: return false;
   }
   return true;
}

bool GLEScmValidate::texParams(GLenum target,GLenum pname) {
    switch(pname) {
    case GL_TEXTURE_MIN_FILTER:
    case GL_TEXTURE_MAG_FILTER:
    case GL_TEXTURE_WRAP_S:
    case GL_TEXTURE_WRAP_T:
    case GL_TEXTURE_CROP_RECT_OES:
    case GL_GENERATE_MIPMAP:
    case GL_TEXTURE_MAX_ANISOTROPY_EXT:
        break;
    default:
        return false;
    }
    return (target == GL_TEXTURE_2D)||(target == GL_TEXTURE_CUBE_MAP_OES);
}

bool GLEScmValidate::texEnv(GLenum target,GLenum pname) {
    switch(pname) {
    case GL_TEXTURE_ENV_MODE:
    case GL_TEXTURE_ENV_COLOR:
    case GL_COMBINE_RGB:
    case GL_COMBINE_ALPHA:
    case GL_SRC0_RGB:
    case GL_SRC1_RGB:
    case GL_SRC2_RGB:
    case GL_SRC0_ALPHA:
    case GL_SRC1_ALPHA:
    case GL_SRC2_ALPHA:
    case GL_OPERAND0_RGB:
    case GL_OPERAND1_RGB:
    case GL_OPERAND2_RGB:
    case GL_OPERAND0_ALPHA:
    case GL_OPERAND1_ALPHA:
    case GL_OPERAND2_ALPHA:
    case GL_RGB_SCALE:
    case GL_ALPHA_SCALE:
    case GL_COORD_REPLACE_OES:
        break;
    default:
        return false;
    }
    return (target == GL_TEXTURE_ENV || target == GL_POINT_SPRITE_OES);
}

bool GLEScmValidate::capability(GLenum cap,int maxLights,int maxClipPlanes) {
    switch(cap) {
    case GL_ALPHA_TEST:
    case GL_BLEND:
    case GL_COLOR_ARRAY:
    case GL_COLOR_LOGIC_OP:
    case GL_COLOR_MATERIAL:
    case GL_CULL_FACE:
    case GL_DEPTH_TEST:
    case GL_DITHER:
    case GL_FOG:
    case GL_LIGHTING:
    case GL_LINE_SMOOTH:
    case GL_MULTISAMPLE:
    case GL_NORMAL_ARRAY:
    case GL_NORMALIZE:
    case GL_POINT_SIZE_ARRAY_OES:
    case GL_POINT_SMOOTH:
    case GL_POINT_SPRITE_OES:
    case GL_POLYGON_OFFSET_FILL:
    case GL_RESCALE_NORMAL:
    case GL_SAMPLE_ALPHA_TO_COVERAGE:
    case GL_SAMPLE_ALPHA_TO_ONE:
    case GL_SAMPLE_COVERAGE:
    case GL_SCISSOR_TEST:
    case GL_STENCIL_TEST:
    case GL_TEXTURE_2D:
    case GL_TEXTURE_COORD_ARRAY:
    case GL_VERTEX_ARRAY:
        return true;
    }
    return GLEScmValidate::lightEnum(cap,maxLights) || GLEScmValidate::clipPlaneEnum(cap,maxClipPlanes);
}


bool GLEScmValidate::texCompImgFrmt(GLenum format) {
    switch(format) {
    case GL_PALETTE4_RGB8_OES:
    case GL_PALETTE4_RGBA8_OES:
    case GL_PALETTE4_R5_G6_B5_OES:
    case GL_PALETTE4_RGBA4_OES:
    case GL_PALETTE4_RGB5_A1_OES:
    case GL_PALETTE8_RGB8_OES:
    case GL_PALETTE8_RGBA8_OES:
    case GL_PALETTE8_R5_G6_B5_OES:
    case GL_PALETTE8_RGBA4_OES:
    case GL_PALETTE8_RGB5_A1_OES:
        return true;
    }
    return false;
}

bool GLEScmValidate::blendDst(GLenum d) {
   switch(d) {
    case GL_ZERO:
    case GL_ONE:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
        return true;
   }
   return false;
}

bool GLEScmValidate::renderbufferInternalFrmt(GLEScontext* ctx, GLenum internalformat)
{
    switch (internalformat) {
    case GL_DEPTH_COMPONENT16_OES:
    case GL_RGBA4_OES:
    case GL_RGB5_A1_OES:
    case GL_RGB565_OES:
    case GL_STENCIL_INDEX1_OES:
    case GL_STENCIL_INDEX4_OES:
    case GL_STENCIL_INDEX8_OES:
    case GL_RGB8_OES:
    case GL_RGBA8_OES:
    case GL_DEPTH_COMPONENT24_OES:
    case GL_DEPTH_COMPONENT32_OES:
        return true;
    }
    if (ctx->getCaps()->GL_EXT_PACKED_DEPTH_STENCIL && internalformat==GL_DEPTH24_STENCIL8_OES)
        return true;

    return false;
}

bool GLEScmValidate::stencilOp(GLenum param) {
    switch (param) {
    case GL_KEEP:
    case GL_ZERO:
    case GL_REPLACE:
    case GL_INCR:
    case GL_DECR:
    case GL_INVERT:
    case GL_INCR_WRAP_OES:
    case GL_DECR_WRAP_OES:
      return true;
    }
    return false;
}

bool GLEScmValidate::texGen(GLenum coord, GLenum pname) {
    return (coord == GL_TEXTURE_GEN_STR_OES && pname == GL_TEXTURE_GEN_MODE_OES);
}

bool GLEScmValidate::colorPointerType(GLenum type){
    switch(type){
    case GL_UNSIGNED_BYTE:
    case GL_FIXED:
    case GL_FLOAT:
        return true;
    }
    return false;
}

bool GLEScmValidate::normalPointerType(GLenum type){

    switch(type){
    case GL_BYTE:
    case GL_SHORT:
    case GL_FLOAT:
    case GL_FIXED:
        return true;
    }
    return false;
}

bool GLEScmValidate::pointPointerType(GLenum type){
    return type == GL_FIXED || type == GL_FLOAT;
}

bool GLEScmValidate::texCoordPointerType(GLenum type){
    switch(type){
    case GL_BYTE:
    case GL_SHORT:
    case GL_FLOAT:
    case GL_FIXED:
        return true;
    }
    return false;
}

bool GLEScmValidate::vertexPointerType(GLenum type){
    switch(type){
    case GL_BYTE:
    case GL_SHORT:
    case GL_FLOAT:
    case GL_FIXED:
        return true;
    }
    return false;
}

