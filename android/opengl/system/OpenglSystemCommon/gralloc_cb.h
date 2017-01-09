/*
* Copyright 2011 The Android Open Source Project
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

#ifndef __GRALLOC_CB_H__
#define __GRALLOC_CB_H__

#include <hardware/hardware.h>
#include <hardware/gralloc.h>
#include <cutils/native_handle.h>

#define BUFFER_HANDLE_MAGIC ((int)0xabfabfab)
#define CB_HANDLE_NUM_INTS(nfds) (int)((sizeof(cb_handle_t) - (nfds)*sizeof(int)) / sizeof(int))

//
// Our buffer handle structure
//
struct cb_handle_t : public native_handle {

    cb_handle_t(int p_fd, int p_ashmemSize, int p_usage,
                int p_width, int p_height, int p_frameworkFormat,
                int p_format, int p_glFormat, int p_glType) :
        fd(p_fd),
        magic(BUFFER_HANDLE_MAGIC),
        usage(p_usage),
        width(p_width),
        height(p_height),
        frameworkFormat(p_frameworkFormat),
        format(p_format),
        glFormat(p_glFormat),
        glType(p_glType),
        ashmemSize(p_ashmemSize),
        ashmemBase(0),
        ashmemBasePid(0),
        mappedPid(0),
        lockedLeft(0),
        lockedTop(0),
        lockedWidth(0),
        lockedHeight(0),
        hostHandle(0)
    {
        version = sizeof(native_handle);
        numFds = 0;
        numInts = CB_HANDLE_NUM_INTS(numFds);
    }

    ~cb_handle_t() {
        magic = 0;
    }

    void setFd(int p_fd) {
        if (p_fd >= 0) {
            numFds = 1;
        }
        else {
            numFds = 0;
        }
        fd = p_fd;
        numInts = CB_HANDLE_NUM_INTS(numFds);
    }

    static bool validate(const cb_handle_t* hnd) {
        return (hnd &&
                hnd->version == sizeof(native_handle) &&
                hnd->magic == BUFFER_HANDLE_MAGIC &&
                hnd->numInts == CB_HANDLE_NUM_INTS(hnd->numFds));
    }

    bool canBePosted() {
        return (0 != (usage & GRALLOC_USAGE_HW_FB));
    }

    // file-descriptors
    int fd;  // ashmem fd (-1 of ashmem region did not allocated, i.e. no SW access needed)

    // ints
    int magic;              // magic number in order to validate a pointer to be a cb_handle_t
    int usage;              // usage bits the buffer was created with
    int width;              // buffer width
    int height;             // buffer height
    int frameworkFormat;    // format requested by the Android framework
    int format;             // real internal pixel format format
    int glFormat;           // OpenGL format enum used for host h/w color buffer
    int glType;             // OpenGL type enum used when uploading to host
    int ashmemSize;         // ashmem region size for the buffer (0 unless is HW_FB buffer or
                            //                                    s/w access is needed)
    union {
        intptr_t ashmemBase;    // CPU address of the mapped ashmem region
        uint64_t padding;       // enforce same size on 32-bit/64-bit
    } __attribute__((aligned(8)));

    int ashmemBasePid;      // process id which mapped the ashmem region
    int mappedPid;          // process id which succeeded gralloc_register call
    int lockedLeft;         // region of buffer locked for s/w write
    int lockedTop;
    int lockedWidth;
    int lockedHeight;
    uint32_t hostHandle;
};


#endif //__GRALLOC_CB_H__
