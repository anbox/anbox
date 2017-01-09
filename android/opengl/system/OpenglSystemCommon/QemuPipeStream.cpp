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
#include "QemuPipeStream.h"
#include <hardware/qemu_pipe.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

QemuPipeStream::QemuPipeStream(size_t bufSize) :
    IOStream(bufSize),
    m_sock(-1),
    m_bufsize(bufSize),
    m_buf(NULL)
{
}

QemuPipeStream::QemuPipeStream(int sock, size_t bufSize) :
    IOStream(bufSize),
    m_sock(sock),
    m_bufsize(bufSize),
    m_buf(NULL)
{
}

QemuPipeStream::~QemuPipeStream()
{
    if (m_sock >= 0) {
        flush();
        ::close(m_sock);
    }
    if (m_buf != NULL) {
        free(m_buf);
    }
}


int QemuPipeStream::connect(void)
{
    m_sock = qemu_pipe_open("opengles");
    if (!valid()) return -1;
    return 0;
}

void *QemuPipeStream::allocBuffer(size_t minSize)
{
    size_t allocSize = (m_bufsize < minSize ? minSize : m_bufsize);
    if (!m_buf) {
        m_buf = (unsigned char *)malloc(allocSize);
    }
    else if (m_bufsize < allocSize) {
        unsigned char *p = (unsigned char *)realloc(m_buf, allocSize);
        if (p != NULL) {
            m_buf = p;
            m_bufsize = allocSize;
        } else {
            ERR("realloc (%d) failed\n", allocSize);
            free(m_buf);
            m_buf = NULL;
            m_bufsize = 0;
        }
    }

    return m_buf;
};

int QemuPipeStream::commitBuffer(size_t size)
{
    return writeFully(m_buf, size);
}

int QemuPipeStream::writeFully(const void *buf, size_t len)
{
    //DBG(">> QemuPipeStream::writeFully %d\n", len);
    if (!valid()) return -1;
    if (!buf) {
       if (len>0) {
            // If len is non-zero, buf must not be NULL. Otherwise the pipe would be
            // in a corrupted state, which is lethal for the emulator.
           ERR("QemuPipeStream::writeFully failed, buf=NULL, len %d,"
                   " lethal error, exiting", len);
           abort();
       }
       return 0;
    }

    size_t res = len;
    int retval = 0;

    while (res > 0) {
        ssize_t stat = ::write(m_sock, (const char *)(buf) + (len - res), res);
        if (stat > 0) {
            res -= stat;
            continue;
        }
        if (stat == 0) { /* EOF */
            ERR("QemuPipeStream::writeFully failed: premature EOF\n");
            retval = -1;
            break;
        }
        if (errno == EINTR) {
            continue;
        }
        retval =  stat;
        ERR("QemuPipeStream::writeFully failed: %s, lethal error, exiting.\n",
                strerror(errno));
        abort();
    }
    //DBG("<< QemuPipeStream::writeFully %d\n", len );
    return retval;
}

const unsigned char *QemuPipeStream::readFully(void *buf, size_t len)
{
    //DBG(">> QemuPipeStream::readFully %d\n", len);
    if (!valid()) return NULL;
    if (!buf) {
        if (len > 0) {
            // If len is non-zero, buf must not be NULL. Otherwise the pipe would be
            // in a corrupted state, which is lethal for the emulator.
            ERR("QemuPipeStream::readFully failed, buf=NULL, len %zu, lethal"
                    " error, exiting.", len);
            abort();
        }
        return NULL;  // do not allow NULL buf in that implementation
    }
    size_t res = len;
    while (res > 0) {
        ssize_t stat = ::read(m_sock, (char *)(buf) + len - res, res);
        if (stat == 0) {
            // client shutdown;
            return NULL;
        } else if (stat < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                ERR("QemuPipeStream::readFully failed (buf %p, len %zu"
                    ", res %zu): %s, lethal error, exiting.", buf, len, res,
                    strerror(errno));
                abort();
            }
        } else {
            res -= stat;
        }
    }
    //DBG("<< QemuPipeStream::readFully %d\n", len);
    return (const unsigned char *)buf;
}

const unsigned char *QemuPipeStream::read( void *buf, size_t *inout_len)
{
    //DBG(">> QemuPipeStream::read %d\n", *inout_len);
    if (!valid()) return NULL;
    if (!buf) {
      ERR("QemuPipeStream::read failed, buf=NULL");
      return NULL;  // do not allow NULL buf in that implementation
    }

    int n = recv(buf, *inout_len);

    if (n > 0) {
        *inout_len = n;
        return (const unsigned char *)buf;
    }

    //DBG("<< QemuPipeStream::read %d\n", *inout_len);
    return NULL;
}

int QemuPipeStream::recv(void *buf, size_t len)
{
    if (!valid()) return int(ERR_INVALID_SOCKET);
    char* p = (char *)buf;
    int ret = 0;
    while(len > 0) {
        int res = ::read(m_sock, p, len);
        if (res > 0) {
            p += res;
            ret += res;
            len -= res;
            continue;
        }
        if (res == 0) { /* EOF */
             break;
        }
        if (errno == EINTR)
            continue;

        /* A real error */
        if (ret == 0)
            ret = -1;
        break;
    }
    return ret;
}
