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
#include <stdio.h>
#include <stdlib.h>
#include "ServerConnection.h"
#include "TcpStream.h"
#include "QemuPipeStream.h"
#include <cutils/log.h>
#include "ThreadInfo.h"

gl_client_context_t *ServerConnection::s_getGlContext()
{
    EGLThreadInfo *ti = getEGLThreadInfo();
    if (ti->serverConn) {
        return ti->serverConn->m_glEnc;
    }
    return NULL;
}

gl2_client_context_t *ServerConnection::s_getGl2Context()
{
    EGLThreadInfo *ti = getEGLThreadInfo();
    if (ti->serverConn) {
        return ti->serverConn->m_gl2Enc;
    }
    return NULL;
}

ServerConnection *ServerConnection::s_getServerConnection()
{
    EGLThreadInfo *ti = getEGLThreadInfo();
    if (!ti->serverConn)
    {
        ti->serverConn = new ServerConnection();
        if (ti->serverConn->create() < 0) {
            delete ti->serverConn;
            ti->serverConn = NULL;
        }
    }

    return ti->serverConn;
}


ServerConnection::ServerConnection() :
    m_stream(NULL),
    m_glEnc(NULL),
    m_ut_enc(NULL)
{
}

ServerConnection::~ServerConnection()
{
    delete m_ut_enc;
    delete m_glEnc;
    delete m_stream;
}



int ServerConnection::create(size_t bufsize,
                             const char *defaultServer)
{
    /* XXX: Make configurable through system property */
    int useQemuPipe = 1;

    if (m_stream != NULL) delete(m_stream);

    if (useQemuPipe) {
        QemuPipeStream*  pipeStream = new QemuPipeStream(bufsize);

        if (pipeStream->connect() < 0) {
            ALOGE("couldn't connect to host server\n");
            delete pipeStream;
            return -1;
        }
        m_stream = pipeStream;
    }
    else /* !useQemuPipe */
    {
        TcpStream*  tcpStream = new TcpStream(bufsize);

        char *s = getenv(ENV_RGL_SERVER);
        char *hostname;
        if (s == NULL) {
            hostname = strdup(defaultServer);
        } else {
            hostname = strdup(s);
        }

        if (tcpStream->connect(hostname, CODEC_SERVER_PORT) < 0) {
            ALOGE("couldn't connect to %s\n", hostname);
            free(hostname);
            delete tcpStream;
            return -1;
        }
        LOGI("connecting to server %s\n", hostname);
        free(hostname);

        m_stream = tcpStream;
    }

    m_glEnc = new GLEncoder(m_stream);
    m_glEnc->setContextAccessor(s_getGlContext);

    m_gl2Enc = new GL2Encoder(m_stream);
    m_gl2Enc->setContextAccessor(s_getGl2Context);

    m_ut_enc = new ut_rendercontrol_encoder_context_t(m_stream);
    return 0;
}

