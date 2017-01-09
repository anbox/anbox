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
#ifndef _FIXED_BUFFER_H
#define _FIXED_BUFFER_H

class FixedBuffer {
public:
    FixedBuffer(size_t initialSize = 0) {
        m_buffer = NULL;
        m_bufferLen = initialSize;
        alloc(m_bufferLen);
    }

    ~FixedBuffer() {
        delete [] m_buffer;
        m_bufferLen = 0;
    }

    void * alloc(size_t size) {
        if (m_bufferLen >= size)
            return (void *)(m_buffer);

        if (m_buffer != NULL)
            delete[] m_buffer;

        m_bufferLen = size;
        m_buffer = new unsigned char[m_bufferLen];
        if (m_buffer == NULL)
            m_bufferLen = 0;

        return m_buffer;
    }
    void *ptr() { return m_buffer; }
    size_t len() { return m_bufferLen; }
private:
    unsigned char *m_buffer;
    size_t m_bufferLen;
};

#endif
