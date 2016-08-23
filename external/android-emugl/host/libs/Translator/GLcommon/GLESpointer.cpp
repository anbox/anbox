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
#include <GLcommon/GLESpointer.h>
#include <stdlib.h>

GLESpointer::GLESpointer():m_size(4),
                           m_type(GL_FLOAT),
                           m_stride(0),
                           m_enabled(false),
                           m_normalize(false),
                           m_data(NULL),
                           m_buffer(NULL),
                           m_bufferName(0),
                           m_buffOffset(0),
                           m_isVBO(false){};


GLenum GLESpointer:: getType() const {
    return m_type;
}

GLint GLESpointer::getSize() const  {
    return m_size;
}

GLsizei GLESpointer::getStride() const {
    return m_stride;
}

const GLvoid* GLESpointer::getArrayData() const {
    return m_data;
}

GLvoid* GLESpointer::getBufferData() const {
    return  m_buffer ? static_cast<unsigned char*>(m_buffer->getData()) + m_buffOffset : NULL;
}

const GLvoid* GLESpointer::getData() const{
    return m_isVBO ? getBufferData():getArrayData();
}

void GLESpointer::redirectPointerData(){
    m_data = getBufferData();
}

GLuint GLESpointer::getBufferName() const {
    return m_bufferName;
}

unsigned int GLESpointer::getBufferOffset() const {

    return  m_buffOffset;
}

bool GLESpointer::isEnable() const {
    return m_enabled;
}

bool GLESpointer::isNormalize() const {
    return m_normalize;
}

bool GLESpointer::isVBO() const {
    return m_isVBO;
}

void GLESpointer::enable(bool b) {
    m_enabled = b;
}

void GLESpointer::setArray(GLint size,GLenum type,GLsizei stride,const GLvoid* data,bool normalize) {
    m_size   = size;
    m_type   = type;
    m_stride = stride;
    m_data   = data;
    m_buffer = NULL;
    m_bufferName = 0;
    m_normalize = normalize;
    m_isVBO = false;
}

void GLESpointer::setBuffer(GLint size,GLenum type,GLsizei stride,GLESbuffer* buf,GLuint bufferName,int offset,bool normalize) {
    m_size   = size;
    m_type   = type;
    m_stride = stride;
    m_data   = NULL;
    m_buffer = buf;
    m_bufferName = bufferName;
    m_buffOffset = offset;
    m_normalize = normalize;
    m_isVBO = true;
}

void GLESpointer::getBufferConversions(const RangeList& rl,RangeList& rlOut) {
    m_buffer->getConversions(rl,rlOut);
}
