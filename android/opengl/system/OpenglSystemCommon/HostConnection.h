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
#ifndef __COMMON_HOST_CONNECTION_H
#define __COMMON_HOST_CONNECTION_H

#include "IOStream.h"
#include "renderControl_enc.h"
#include "ChecksumCalculator.h"

class GLEncoder;
class gl_client_context_t;
class GL2Encoder;
class gl2_client_context_t;

class HostConnection
{
public:
    static HostConnection *get();
    ~HostConnection();

    GLEncoder *glEncoder();
    GL2Encoder *gl2Encoder();
    renderControl_encoder_context_t *rcEncoder();
    ChecksumCalculator *checksumHelper() { return &m_checksumHelper; }

    void flush() {
        if (m_stream) {
            m_stream->flush();
        }
    }

private:
    HostConnection();
    static gl_client_context_t  *s_getGLContext();
    static gl2_client_context_t *s_getGL2Context();
    // setProtocol initilizes GL communication protocol for checksums
    // should be called when m_rcEnc is created
    void setChecksumHelper(renderControl_encoder_context_t *rcEnc);

private:
    IOStream *m_stream;
    GLEncoder   *m_glEnc;
    GL2Encoder  *m_gl2Enc;
    renderControl_encoder_context_t *m_rcEnc;
    ChecksumCalculator m_checksumHelper;
};

#endif
