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
#include "Win32PipeStream.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <windows.h>

#ifndef _WIN32
#error ONLY BUILD THIS SOURCE FILE FOR WINDOWS!
#endif

/* The official documentation states that the name of a given named
 * pipe cannot be more than 256 characters long.
 */
#define NAMED_PIPE_MAX 256

Win32PipeStream::Win32PipeStream(size_t bufSize) :
    SocketStream(bufSize),
    m_pipe(INVALID_HANDLE_VALUE)
{
}

Win32PipeStream::Win32PipeStream(HANDLE pipe, size_t bufSize) :
    SocketStream(-1, bufSize),
    m_pipe(pipe)
{
}

Win32PipeStream::~Win32PipeStream()
{
    if (m_pipe != INVALID_HANDLE_VALUE) {
        CloseHandle(m_pipe);
        m_pipe = INVALID_HANDLE_VALUE;
    }
}

/* Initialize the pipe name corresponding to a given port
 */
static void
make_pipe_name(char *path, size_t  pathlen, int port_number)
{
    snprintf(path, pathlen, "\\\\.\\pipe\\qemu-gles-%d", port_number);
}


/* Technical note: Named pipes work differently from BSD Sockets.
 * One does not create/bind a pipe, and collect a new handle each
 * time a client connects with accept().
 *
 * Instead, the server creates a new pipe instance each time it wants
 * to get a new client connection, then calls ConnectNamedPipe() to
 * wait for a connection.
 *
 * So listen() is a no-op, and accept() really creates the pipe handle.
 *
 * Also, connect() must create a pipe handle with CreateFile() and
 * wait for a server instance with WaitNamedPipe()
 */
int Win32PipeStream::listen(char addrstr[MAX_ADDRSTR_LEN])
{
    m_port = GetCurrentProcessId();
    make_pipe_name(addrstr, MAX_ADDRSTR_LEN, m_port);
    return 0;
}

SocketStream * Win32PipeStream::accept()
{
    char path[NAMED_PIPE_MAX+1];
    SocketStream*  clientStream;
    HANDLE pipe;

    make_pipe_name(path, sizeof(path), m_port);

    pipe = ::CreateNamedPipe(
                path,                // pipe name
                PIPE_ACCESS_DUPLEX,  // read-write access
                PIPE_TYPE_BYTE |     // byte-oriented writes
                PIPE_READMODE_BYTE | // byte-oriented reads
                PIPE_WAIT,           // blocking operations
                PIPE_UNLIMITED_INSTANCES, // no limit on clients
                4096,                // input buffer size
                4096,                // output buffer size
                0,                   // client time-out
                NULL);               // default security attributes

    if (pipe == INVALID_HANDLE_VALUE) {
        ERR("%s: CreateNamedPipe failed %d\n", __FUNCTION__, (int)GetLastError());
        return NULL;
    }

    // Stupid Win32 API design: If a client is already connected, then
    // ConnectNamedPipe will return 0, and GetLastError() will return
    // ERROR_PIPE_CONNECTED. This is not an error! It just means that the
    // function didn't have to wait.
    //
    if (::ConnectNamedPipe(pipe, NULL) == 0 && GetLastError() != ERROR_PIPE_CONNECTED) {
        ERR("%s: ConnectNamedPipe failed: %d\n", __FUNCTION__, (int)GetLastError());
        CloseHandle(pipe);
        return NULL;
    }

    clientStream = new Win32PipeStream(pipe, m_bufsize);
    return clientStream;
}

int Win32PipeStream::connect(const char* addr)
{
    HANDLE pipe;
    int    tries = 10;

    /* We're going to loop in order to wait for the pipe server to
     * be setup properly.
     */
    for (; tries > 0; tries--) {
        pipe = ::CreateFile(
                    addr,                          // pipe name
                    GENERIC_READ | GENERIC_WRITE,  // read & write
                    0,                             // no sharing
                    NULL,                          // default security attrs
                    OPEN_EXISTING,                 // open existing pipe
                    0,                             // default attributes
                    NULL);                         // no template file

        /* If we have a valid pipe handle, break from the loop */
        if (pipe != INVALID_HANDLE_VALUE) {
            break;
        }

        /* We can get here if the pipe is busy, i.e. if the server hasn't
         * create a new pipe instance to service our request. In which case
         * GetLastError() will return ERROR_PIPE_BUSY.
         *
         * If so, then use WaitNamedPipe() to wait for a decent time
         * to try again.
         */
        if (GetLastError() != ERROR_PIPE_BUSY) {
            /* Not ERROR_PIPE_BUSY */
            ERR("%s: CreateFile failed: %d\n", __FUNCTION__, (int)GetLastError());
            errno = EINVAL;
            return -1;
        }

        /* Wait for 5 seconds */
        if ( !WaitNamedPipe(addr, 5000) ) {
            ERR("%s: WaitNamedPipe failed: %d\n", __FUNCTION__, (int)GetLastError());
            errno = EINVAL;
            return -1;
        }
    }

    m_pipe = pipe;
    return 0;
}

/* Special buffer methods, since we can't use socket functions here */

int Win32PipeStream::commitBuffer(size_t size)
{
    if (m_pipe == INVALID_HANDLE_VALUE)
        return -1;

    size_t res = size;
    int retval = 0;

    while (res > 0) {
        DWORD  written;
        if (! ::WriteFile(m_pipe, (const char *)m_buf + (size - res), res, &written, NULL)) {
            retval =  -1;
            ERR("%s: failed: %d\n", __FUNCTION__, (int)GetLastError());
            break;
        }
        res -= written;
    }
    return retval;
}

const unsigned char *Win32PipeStream::readFully(void *buf, size_t len)
{
    if (m_pipe == INVALID_HANDLE_VALUE)
        return NULL;

    if (!buf) {
        return NULL;  // do not allow NULL buf in that implementation
    }

    size_t res = len;
    while (res > 0) {
        DWORD  readcount = 0;
        if (! ::ReadFile(m_pipe, (char *)buf + (len - res), res, &readcount, NULL) || readcount == 0) {
            errno = (int)GetLastError();
            return NULL;
        }
        res -= readcount;
    }
    return (const unsigned char *)buf;
}

const unsigned char *Win32PipeStream::read( void *buf, size_t *inout_len)
{
    size_t len = *inout_len;
    DWORD  readcount;

    if (m_pipe == INVALID_HANDLE_VALUE)
        return NULL;

    if (!buf) {
        return NULL;  // do not allow NULL buf in that implementation
    }

    if (!::ReadFile(m_pipe, (char *)buf, len, &readcount, NULL)) {
        errno = (int)GetLastError();
        return NULL;
    }

    *inout_len = (size_t)readcount;
    return (const unsigned char *)buf;
}

void Win32PipeStream::forceStop()
{
    HANDLE handle = m_pipe;
    m_pipe = INVALID_HANDLE_VALUE;
    CloseHandle(handle);
}
