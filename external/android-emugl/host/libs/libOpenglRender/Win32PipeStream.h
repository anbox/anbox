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
#ifndef __WIN32_PIPE_STREAM_H
#define __WIN32_PIPE_STREAM_H

#include "SocketStream.h"
#include <windows.h>

class Win32PipeStream : public SocketStream {
public:
    explicit Win32PipeStream(size_t bufsize = 10000);
    virtual ~Win32PipeStream();
    virtual int listen(char addrstr[MAX_ADDRSTR_LEN]);
    virtual SocketStream *accept();
    virtual int connect(const char* addr);

    virtual int commitBuffer(size_t size);
    virtual const unsigned char *readFully(void *buf, size_t len);
    virtual const unsigned char *read(void *buf, size_t *inout_len);
    virtual void forceStop();

private:
    Win32PipeStream(HANDLE pipe, size_t bufSize);
    HANDLE  m_pipe;
    int     m_port;
};


#endif
