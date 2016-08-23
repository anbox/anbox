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
#include <GLcommon/GLESbuffer.h>
#include <string.h>

bool  GLESbuffer::setBuffer(GLuint size,GLuint usage,const GLvoid* data) {
    m_size = size;
    m_usage = usage;
    if(m_data) {
        delete [] m_data;
        m_data = NULL;
    }
    m_data = new unsigned char[size];
    if(m_data) {
        if(data) {
            memcpy(m_data,data,size);
        }
        m_conversionManager.clear();
        m_conversionManager.addRange(Range(0,m_size));
        return true;
    }
    return false;
}

bool  GLESbuffer::setSubBuffer(GLint offset,GLuint size,const GLvoid* data) {
    if(offset + size > m_size) return false;
    memcpy(m_data+offset,data,size);
    m_conversionManager.addRange(Range(offset,size));
    m_conversionManager.merge();
    return true;
}

void  GLESbuffer::getConversions(const RangeList& rIn,RangeList& rOut) {
        m_conversionManager.delRanges(rIn,rOut);
        rOut.merge();
}

GLESbuffer::~GLESbuffer() {
    if(m_data) {
        delete [] m_data;
    }
}
