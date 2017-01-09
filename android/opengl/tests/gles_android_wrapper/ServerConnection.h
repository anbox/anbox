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
#ifndef _SERVER_CONNECTION_H
#define _SERVER_CONNECTION_H

#include "GLEncoder.h"
#include "GL2Encoder.h"
#include "IOStream.h"
#include "codec_defs.h"
#include "ut_rendercontrol_enc.h"
#include <pthread.h>

#define ENV_RGL_SERVER "RGL_SERVER"
#define RGL_DEFAULT_SERVER "10.0.2.2"

class ServerConnection {
public:
    ~ServerConnection();
    int create(size_t buf_size = 4 * 1024 * 1024, const char *defaultServer = RGL_DEFAULT_SERVER);
    static gl_client_context_t *s_getGlContext();
    static ServerConnection *s_getServerConnection();
    static gl2_client_context_t *s_getGl2Context();
    GLEncoder *glEncoder() { return m_glEnc; }
    GL2Encoder *gl2Encoder() { return m_gl2Enc; }
    ut_rendercontrol_encoder_context_t * utEnc() { return m_ut_enc; }

private:
    ServerConnection();

private:
    static pthread_key_t s_glKey;
    static pthread_key_t s_connectionKey;
    static void s_initKeys();
    IOStream *m_stream;
    GLEncoder *m_glEnc;
    GL2Encoder *m_gl2Enc;
    ut_rendercontrol_encoder_context_t *m_ut_enc;

};


#endif
