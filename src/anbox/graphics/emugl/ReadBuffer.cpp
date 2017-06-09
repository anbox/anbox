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

#include "anbox/graphics/emugl/ReadBuffer.h"
#include "anbox/logger.h"

#include <assert.h>
#include <limits.h>
#include <string.h>

ReadBuffer::ReadBuffer(size_t bufsize) {
  m_size = bufsize;
  m_buf = static_cast<unsigned char*>(malloc(m_size * sizeof(unsigned char)));
  m_validData = 0;
  m_readPtr = m_buf;
}

ReadBuffer::~ReadBuffer() { free(m_buf); }

int ReadBuffer::getData(IOStream* stream) {
  if (stream == NULL) return -1;
  if ((m_validData > 0) && (m_readPtr > m_buf)) {
    memmove(m_buf, m_readPtr, m_validData);
  }
  // get fresh data into the buffer;
  size_t len = m_size - m_validData;
  if (len == 0) {
    // we need to inc our buffer
    size_t new_size = m_size * 2;
    unsigned char* new_buf;
    if (new_size < m_size) {  // overflow check
      new_size = INT_MAX;
    }

    new_buf = static_cast<unsigned char*>(realloc(m_buf, new_size));
    if (!new_buf) {
      ERROR("Failed to alloc %zu bytes for ReadBuffer", new_size);
      return -1;
    }
    m_size = new_size;
    m_buf = new_buf;
    len = m_size - m_validData;
  }
  m_readPtr = m_buf;
  if (NULL != stream->read(m_buf + m_validData, &len)) {
    m_validData += len;
    return len;
  }
  return -1;
}

void ReadBuffer::consume(size_t amount) {
  assert(amount <= m_validData);
  m_validData -= amount;
  m_readPtr += amount;
}
