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
#include "HostConnection.h"
#include "TcpStream.h"
#include "QemuPipeStream.h"
#include "ThreadInfo.h"
#include <cutils/log.h>
#include "GLEncoder.h"
#include "GL2Encoder.h"
#include <memory>

#define STREAM_BUFFER_SIZE  4*1024*1024
#define STREAM_PORT_NUM     22468

/* Set to 1 to use a QEMU pipe, or 0 for a TCP connection */
#define  USE_QEMU_PIPE  1

HostConnection::HostConnection() :
    m_stream(NULL),
    m_glEnc(NULL),
    m_gl2Enc(NULL),
    m_rcEnc(NULL),
    m_checksumHelper()
{
}

HostConnection::~HostConnection()
{
    delete m_stream;
    delete m_glEnc;
    delete m_gl2Enc;
    delete m_rcEnc;
}

HostConnection *HostConnection::get()
{
    /* TODO: Make this configurable with a system property */
    const int useQemuPipe = USE_QEMU_PIPE;

    // Get thread info
    EGLThreadInfo *tinfo = getEGLThreadInfo();
    if (!tinfo) {
        return NULL;
    }

    if (tinfo->hostConn == NULL) {
        HostConnection *con = new HostConnection();
        if (NULL == con) {
            return NULL;
        }

        if (useQemuPipe) {
            QemuPipeStream *stream = new QemuPipeStream(STREAM_BUFFER_SIZE);
            if (!stream) {
                ALOGE("Failed to create QemuPipeStream for host connection!!!\n");
                delete con;
                return NULL;
            }
            if (stream->connect() < 0) {
                ALOGE("Failed to connect to host (QemuPipeStream)!!!\n");
                delete stream;
                delete con;
                return NULL;
            }
            con->m_stream = stream;
        }
        else /* !useQemuPipe */
        {
            TcpStream *stream = new TcpStream(STREAM_BUFFER_SIZE);
            if (!stream) {
                ALOGE("Failed to create TcpStream for host connection!!!\n");
                delete con;
                return NULL;
            }

            if (stream->connect("10.0.2.2", STREAM_PORT_NUM) < 0) {
                ALOGE("Failed to connect to host (TcpStream)!!!\n");
                delete stream;
                delete con;
                return NULL;
            }
            con->m_stream = stream;
        }

        // send zero 'clientFlags' to the host.
        unsigned int *pClientFlags =
                (unsigned int *)con->m_stream->allocBuffer(sizeof(unsigned int));
        *pClientFlags = 0;
        con->m_stream->commitBuffer(sizeof(unsigned int));

        ALOGD("HostConnection::get() New Host Connection established %p, tid %d\n", con, gettid());
        tinfo->hostConn = con;
    }

    return tinfo->hostConn;
}

GLEncoder *HostConnection::glEncoder()
{
    if (!m_glEnc) {
        m_glEnc = new GLEncoder(m_stream, checksumHelper());
        DBG("HostConnection::glEncoder new encoder %p, tid %d", m_glEnc, gettid());
        m_glEnc->setContextAccessor(s_getGLContext);
    }
    return m_glEnc;
}

GL2Encoder *HostConnection::gl2Encoder()
{
    if (!m_gl2Enc) {
        m_gl2Enc = new GL2Encoder(m_stream, checksumHelper());
        DBG("HostConnection::gl2Encoder new encoder %p, tid %d", m_gl2Enc, gettid());
        m_gl2Enc->setContextAccessor(s_getGL2Context);
    }
    return m_gl2Enc;
}

renderControl_encoder_context_t *HostConnection::rcEncoder()
{
    if (!m_rcEnc) {
        m_rcEnc = new renderControl_encoder_context_t(m_stream, checksumHelper());
        // TODO: disable checksum as a workaround in a glTexSubImage2D problem
        // Uncomment the following line when the root cause is solved
        //setChecksumHelper(m_rcEnc);
    }
    return m_rcEnc;
}

gl_client_context_t *HostConnection::s_getGLContext()
{
    EGLThreadInfo *ti = getEGLThreadInfo();
    if (ti->hostConn) {
        return ti->hostConn->m_glEnc;
    }
    return NULL;
}

gl2_client_context_t *HostConnection::s_getGL2Context()
{
    EGLThreadInfo *ti = getEGLThreadInfo();
    if (ti->hostConn) {
        return ti->hostConn->m_gl2Enc;
    }
    return NULL;
}

void HostConnection::setChecksumHelper(renderControl_encoder_context_t *rcEnc) {
    std::unique_ptr<char[]> glExtensions;
    int extensionSize = rcEnc->rcGetGLString(rcEnc, GL_EXTENSIONS, NULL, 0);
    if (extensionSize < 0) {
        glExtensions = std::unique_ptr<char[]>(new char[-extensionSize]);
        extensionSize = rcEnc->rcGetGLString(rcEnc, GL_EXTENSIONS, glExtensions.get(), -extensionSize);
        if (extensionSize <= 0) {
            glExtensions.reset();
        }
    }
    // check the host supported version
    uint32_t checksumVersion = 0;
    const char* checksumPrefix = ChecksumCalculator::getMaxVersionStrPrefix();
    const char* glProtocolStr = glExtensions.get() ?
            strstr(glExtensions.get(), checksumPrefix) : NULL;
    if (glProtocolStr) {
        uint32_t maxVersion = ChecksumCalculator::getMaxVersion();
        sscanf(glProtocolStr+strlen(checksumPrefix), "%d", &checksumVersion);
        if (maxVersion < checksumVersion) {
            checksumVersion = maxVersion;
        }
        // The ordering of the following two commands matters!
        // Must tell the host first before setting it in the guest
        rcEnc->rcSelectChecksumCalculator(rcEnc, checksumVersion, 0);
        m_checksumHelper.setVersion(checksumVersion);
    }
}
