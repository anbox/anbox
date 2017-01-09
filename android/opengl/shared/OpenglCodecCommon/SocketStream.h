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
#ifndef __SOCKET_STREAM_H
#define __SOCKET_STREAM_H

#include <stdlib.h>
#include "IOStream.h"

class SocketStream : public IOStream {
public:
    typedef enum { ERR_INVALID_SOCKET = -1000 } SocketStreamError;

    explicit SocketStream(size_t bufsize = 10000);
    virtual ~SocketStream();

    virtual int listen(unsigned short port) = 0;
    virtual SocketStream *accept() = 0;
    virtual int connect(unsigned short port) = 0;

    virtual void *allocBuffer(size_t minSize);
    virtual int commitBuffer(size_t size);
    virtual const unsigned char *readFully(void *buf, size_t len);
    virtual const unsigned char *read(void *buf, size_t *inout_len);

    bool valid() { return m_sock >= 0; }
    virtual int recv(void *buf, size_t len);
    virtual int writeFully(const void *buf, size_t len);

protected:
    int            m_sock;
    size_t         m_bufsize;
    unsigned char *m_buf;

    SocketStream(int sock, size_t bufSize);
};

#endif /* __SOCKET_STREAM_H */
