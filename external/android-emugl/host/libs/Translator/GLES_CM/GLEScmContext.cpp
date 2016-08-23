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

#include "GLEScmContext.h"
#include "GLEScmUtils.h"
#include <GLcommon/GLutils.h>
#include <GLcommon/GLconversion_macros.h>
#include <string.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

void GLEScmContext::init(GlLibrary* glLib) {
    emugl::Mutex::AutoLock mutex(s_lock);
    if(!m_initialized) {
        s_glDispatch.dispatchFuncs(GLES_1_1, glLib);
        GLEScontext::init(glLib);

        m_texCoords = new GLESpointer[s_glSupport.maxTexUnits];
        m_map[GL_TEXTURE_COORD_ARRAY]  = &m_texCoords[m_clientActiveTexture];

        buildStrings((const char*)dispatcher().glGetString(GL_VENDOR),
                     (const char*)dispatcher().glGetString(GL_RENDERER),
                     (const char*)dispatcher().glGetString(GL_VERSION),
                     "OpenGL ES-CM 1.1");
    }
    m_initialized = true;
}

GLEScmContext::GLEScmContext():GLEScontext(),m_texCoords(NULL),m_pointsIndex(-1), m_clientActiveTexture(0) {

    m_map[GL_COLOR_ARRAY]          = new GLESpointer();
    m_map[GL_NORMAL_ARRAY]         = new GLESpointer();
    m_map[GL_VERTEX_ARRAY]         = new GLESpointer();
    m_map[GL_POINT_SIZE_ARRAY_OES] = new GLESpointer();
}


void GLEScmContext::setActiveTexture(GLenum tex) {
   m_activeTexture = tex - GL_TEXTURE0;
}

void GLEScmContext::setClientActiveTexture(GLenum tex) {
   m_clientActiveTexture = tex - GL_TEXTURE0;
   m_map[GL_TEXTURE_COORD_ARRAY] = &m_texCoords[m_clientActiveTexture];
}

GLEScmContext::~GLEScmContext(){
    if(m_texCoords){
        delete[] m_texCoords;
        m_texCoords = NULL;
    }
    m_map[GL_TEXTURE_COORD_ARRAY] = NULL;
}


//setting client side arr
void GLEScmContext::setupArr(const GLvoid* arr,GLenum arrayType,GLenum dataType,GLint size,GLsizei stride,GLboolean normalized, int index){
    if( arr == NULL) return;
    switch(arrayType) {
        case GL_VERTEX_ARRAY:
            s_glDispatch.glVertexPointer(size,dataType,stride,arr);
            break;
        case GL_NORMAL_ARRAY:
            s_glDispatch.glNormalPointer(dataType,stride,arr);
            break;
        case GL_TEXTURE_COORD_ARRAY:
            s_glDispatch.glTexCoordPointer(size,dataType,stride,arr);
            break;
        case GL_COLOR_ARRAY:
            s_glDispatch.glColorPointer(size,dataType,stride,arr);
            break;
        case GL_POINT_SIZE_ARRAY_OES:
            m_pointsIndex = index;
            break;
    }
}


void GLEScmContext::setupArrayPointerHelper(GLESConversionArrays& cArrs,GLint first,GLsizei count,GLenum type,const GLvoid* indices,bool direct,GLenum array_id,GLESpointer* p){
        unsigned int size = p->getSize();
        GLenum dataType = p->getType();

        if(needConvert(cArrs,first,count,type,indices,direct,p,array_id)){
            //conversion has occured
            ArrayData currentArr = cArrs.getCurrentArray();
            setupArr(currentArr.data,array_id,currentArr.type,size,currentArr.stride,GL_FALSE, cArrs.getCurrentIndex());
            ++cArrs;
        } else {
            setupArr(p->getData(),array_id,dataType,size,p->getStride(), GL_FALSE);
        }
}

void GLEScmContext::setupArraysPointers(GLESConversionArrays& cArrs,GLint first,GLsizei count,GLenum type,const GLvoid* indices,bool direct) {
    ArraysMap::iterator it;
    m_pointsIndex = -1;

    //going over all clients arrays Pointers
    for ( it=m_map.begin() ; it != m_map.end(); it++ ) {

        GLenum array_id   = (*it).first;
        GLESpointer* p = (*it).second;
        if(!isArrEnabled(array_id)) continue;
        if(array_id == GL_TEXTURE_COORD_ARRAY) continue; //handling textures later
        setupArrayPointerHelper(cArrs,first,count,type,indices,direct,array_id,p);
    }

    unsigned int activeTexture = m_clientActiveTexture + GL_TEXTURE0;

    s_lock.lock();
    int maxTexUnits = s_glSupport.maxTexUnits;
    s_lock.unlock();

    //converting all texture coords arrays
    for(int i=0; i< maxTexUnits;i++) {

        unsigned int tex = GL_TEXTURE0+i;
        setClientActiveTexture(tex);
        s_glDispatch.glClientActiveTexture(tex);

        GLenum array_id   = GL_TEXTURE_COORD_ARRAY;
        GLESpointer* p = m_map[array_id];
        if(!isArrEnabled(array_id)) continue;
        setupArrayPointerHelper(cArrs,first,count,type,indices,direct,array_id,p);
    }

    setClientActiveTexture(activeTexture);
    s_glDispatch.glClientActiveTexture(activeTexture);
}

void  GLEScmContext::drawPointsData(GLESConversionArrays& cArrs,GLint first,GLsizei count,GLenum type,const GLvoid* indices_in,bool isElemsDraw) {
    const char  *pointsArr =  NULL;
    int stride = 0;
    GLESpointer* p = m_map[GL_POINT_SIZE_ARRAY_OES];

    //choosing the right points sizes array source
    if(m_pointsIndex >= 0) { //point size array was converted
        pointsArr = (const char*)(cArrs[m_pointsIndex].data);
        stride = cArrs[m_pointsIndex].stride;
    } else {
        pointsArr = static_cast<const char*>(p->getData());
        stride = p->getStride();
    }

    if(stride == 0){
        stride = sizeof(GLfloat);
    }


    if(isElemsDraw) {
        int tSize = 0;
        switch (type) {
            case GL_UNSIGNED_BYTE:
                tSize = 1;
                break;
            case GL_UNSIGNED_SHORT:
                tSize = 2;
                break;
            case GL_UNSIGNED_INT:
                tSize = 4;
                break;
        };

        int i = 0;
        while(i<count)
        {
            int sStart = i;
            int sCount = 1;

#define INDEX \
                (type == GL_UNSIGNED_INT ? \
                static_cast<const GLuint*>(indices_in)[i]: \
                type == GL_UNSIGNED_SHORT ? \
                static_cast<const GLushort*>(indices_in)[i]: \
                static_cast<const GLubyte*>(indices_in)[i])

            GLfloat pSize = *((GLfloat*)(pointsArr+(INDEX*stride)));
            i++;

            while(i < count && pSize == *((GLfloat*)(pointsArr+(INDEX*stride))))
            {
                sCount++;
                i++;
            }

            s_glDispatch.glPointSize(pSize);
            s_glDispatch.glDrawElements(GL_POINTS, sCount, type, (char*)indices_in+sStart*tSize);
        }
    } else {
        int i = 0;
        while(i<count)
        {
            int sStart = i;
            int sCount = 1;
            GLfloat pSize = *((GLfloat*)(pointsArr+((first+i)*stride)));
            i++;

            while(i < count && pSize == *((GLfloat*)(pointsArr+((first+i)*stride))))
            {
                sCount++;
                i++;
            }

            s_glDispatch.glPointSize(pSize);
            s_glDispatch.glDrawArrays(GL_POINTS, first+sStart, sCount);
        }
    }
}

void  GLEScmContext::drawPointsArrs(GLESConversionArrays& arrs,GLint first,GLsizei count) {
    drawPointsData(arrs,first,count,0,NULL,false);
}

void GLEScmContext::drawPointsElems(GLESConversionArrays& arrs,GLsizei count,GLenum type,const GLvoid* indices_in) {
    drawPointsData(arrs,0,count,type,indices_in,true);
}

bool GLEScmContext::needConvert(GLESConversionArrays& cArrs,GLint first,GLsizei count,GLenum type,const GLvoid* indices,bool direct,GLESpointer* p,GLenum array_id) {

    bool usingVBO = p->isVBO();
    GLenum arrType = p->getType();
    /*
     conversion is not necessary in the following cases:
      (*) array type is byte but it is not vertex or texture array
      (*) array type is not fixed
    */
    if((arrType != GL_FIXED) && (arrType != GL_BYTE)) return false;
    if((arrType == GL_BYTE   && (array_id != GL_VERTEX_ARRAY)) &&
       (arrType == GL_BYTE   && (array_id != GL_TEXTURE_COORD_ARRAY)) ) return false;


    bool byteVBO = (arrType == GL_BYTE) && usingVBO;
    if(byteVBO){
        p->redirectPointerData();
    }

    if(!usingVBO || byteVBO) {
        if (direct) {
            convertDirect(cArrs,first,count,array_id,p);
        } else {
            convertIndirect(cArrs,count,type,indices,array_id,p);
        }
    } else {
        if (direct) {
            convertDirectVBO(cArrs,first,count,array_id,p) ;
        } else {
            convertIndirectVBO(cArrs,count,type,indices,array_id,p);
        }
    }
    return true;
}

const GLESpointer* GLEScmContext::getPointer(GLenum arrType) {
    GLenum type =
        arrType == GL_VERTEX_ARRAY_POINTER          ? GL_VERTEX_ARRAY :
        arrType == GL_NORMAL_ARRAY_POINTER          ? GL_NORMAL_ARRAY :
        arrType == GL_TEXTURE_COORD_ARRAY_POINTER   ? GL_TEXTURE_COORD_ARRAY :
        arrType == GL_COLOR_ARRAY_POINTER           ? GL_COLOR_ARRAY :
        arrType == GL_POINT_SIZE_ARRAY_POINTER_OES  ? GL_POINT_SIZE_ARRAY_OES :
        0;
    if(type != 0)
    {
        return GLEScontext::getPointer(type);
    }
    return NULL;
}

void GLEScmContext::initExtensionString() {
    *s_glExtensions = "GL_OES_blend_func_separate GL_OES_blend_equation_separate GL_OES_blend_subtract "
                      "GL_OES_byte_coordinates GL_OES_compressed_paletted_texture GL_OES_point_size_array "
                      "GL_OES_point_sprite GL_OES_single_precision GL_OES_stencil_wrap GL_OES_texture_env_crossbar "
                      "GL_OES_texture_mirored_repeat GL_OES_EGL_image GL_OES_element_index_uint GL_OES_draw_texture "
                      "GL_OES_texture_cube_map GL_OES_draw_texture ";
    if (s_glSupport.GL_OES_READ_FORMAT)
        *s_glExtensions+="GL_OES_read_format ";
    if (s_glSupport.GL_EXT_FRAMEBUFFER_OBJECT) {
        *s_glExtensions+="GL_OES_framebuffer_object GL_OES_depth24 GL_OES_depth32 GL_OES_fbo_render_mipmap "
                         "GL_OES_rgb8_rgba8 GL_OES_stencil1 GL_OES_stencil4 GL_OES_stencil8 ";
    }
    if (s_glSupport.GL_EXT_PACKED_DEPTH_STENCIL)
        *s_glExtensions+="GL_OES_packed_depth_stencil ";
    if (s_glSupport.GL_EXT_TEXTURE_FORMAT_BGRA8888)
        *s_glExtensions+="GL_EXT_texture_format_BGRA8888 GL_APPLE_texture_format_BGRA8888 ";
    if (s_glSupport.GL_ARB_MATRIX_PALETTE && s_glSupport.GL_ARB_VERTEX_BLEND) {
        *s_glExtensions+="GL_OES_matrix_palette ";
        GLint max_palette_matrices=0;
        GLint max_vertex_units=0;
        dispatcher().glGetIntegerv(GL_MAX_PALETTE_MATRICES_OES,&max_palette_matrices);
        dispatcher().glGetIntegerv(GL_MAX_VERTEX_UNITS_OES,&max_vertex_units);
        if (max_palette_matrices>=32 && max_vertex_units>=4)
            *s_glExtensions+="GL_OES_extended_matrix_palette ";
    }
    *s_glExtensions+="GL_OES_compressed_ETC1_RGB8_texture ";
}

int GLEScmContext::getMaxTexUnits() {
    return getCaps()->maxTexUnits;
}

bool GLEScmContext::glGetBooleanv(GLenum pname, GLboolean *params)
{
    GLint iParam;

    if(glGetIntegerv(pname, &iParam))
    {
        *params = (iParam != 0);
        return true;
    }

    return false;
}

bool GLEScmContext::glGetFixedv(GLenum pname, GLfixed *params)
{
    GLint iParam;

    if(glGetIntegerv(pname, &iParam))
    {
        *params = I2X(iParam);
        return true;
    }

    return false;
}

bool GLEScmContext::glGetFloatv(GLenum pname, GLfloat *params)
{
    GLint iParam;

    if(glGetIntegerv(pname, &iParam))
    {
        *params = (GLfloat)iParam;
        return true;
    }

    return false;
}

bool GLEScmContext::glGetIntegerv(GLenum pname, GLint *params)
{
    if(GLEScontext::glGetIntegerv(pname, params))
        return true;

    const GLESpointer* ptr = NULL;

    switch(pname){
        case GL_VERTEX_ARRAY_BUFFER_BINDING:
        case GL_VERTEX_ARRAY_SIZE:
        case GL_VERTEX_ARRAY_STRIDE:
        case GL_VERTEX_ARRAY_TYPE:
            ptr = getPointer(GL_VERTEX_ARRAY_POINTER);
            break;

        case GL_NORMAL_ARRAY_BUFFER_BINDING:
        case GL_NORMAL_ARRAY_STRIDE:
        case GL_NORMAL_ARRAY_TYPE:
            ptr = getPointer(GL_NORMAL_ARRAY_POINTER);
            break;

        case GL_COLOR_ARRAY_BUFFER_BINDING:
        case GL_COLOR_ARRAY_SIZE:
        case GL_COLOR_ARRAY_STRIDE:
        case GL_COLOR_ARRAY_TYPE:
            ptr = getPointer(GL_COLOR_ARRAY_POINTER);
            break;

        case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
        case GL_TEXTURE_COORD_ARRAY_SIZE:
        case GL_TEXTURE_COORD_ARRAY_STRIDE:
        case GL_TEXTURE_COORD_ARRAY_TYPE:
            ptr = getPointer(GL_TEXTURE_COORD_ARRAY_POINTER);
            break;

        case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
        case GL_POINT_SIZE_ARRAY_STRIDE_OES:
        case GL_POINT_SIZE_ARRAY_TYPE_OES:
            ptr = getPointer(GL_POINT_SIZE_ARRAY_POINTER_OES);
            break;

        default:
            return false;
    }

    switch(pname)
    {
        case GL_VERTEX_ARRAY_BUFFER_BINDING:
        case GL_NORMAL_ARRAY_BUFFER_BINDING:
        case GL_COLOR_ARRAY_BUFFER_BINDING:
        case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING:
        case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
            *params = ptr ? ptr->getBufferName() : 0;
            break;

        case GL_VERTEX_ARRAY_STRIDE:
        case GL_NORMAL_ARRAY_STRIDE:
        case GL_COLOR_ARRAY_STRIDE:
        case GL_TEXTURE_COORD_ARRAY_STRIDE:
        case GL_POINT_SIZE_ARRAY_STRIDE_OES:
            *params = ptr ? ptr->getStride() : 0;
            break;

        case GL_VERTEX_ARRAY_SIZE:
        case GL_COLOR_ARRAY_SIZE:
        case GL_TEXTURE_COORD_ARRAY_SIZE:
            *params = ptr ? ptr->getSize() : 0;
            break;

        case GL_VERTEX_ARRAY_TYPE:
        case GL_NORMAL_ARRAY_TYPE:
        case GL_COLOR_ARRAY_TYPE:
        case GL_TEXTURE_COORD_ARRAY_TYPE:
        case GL_POINT_SIZE_ARRAY_TYPE_OES:
            *params = ptr ? ptr->getType() : 0;
            break;
    }

    return true;
}
