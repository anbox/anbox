/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Contains implementation of a class NV21JpegCompressor that encapsulates a
 * converter between NV21, and JPEG formats.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera_JPEG"
#include <cutils/log.h>
#include <assert.h>
#include <dlfcn.h>
#include "JpegCompressor.h"

namespace android {

void* NV21JpegCompressor::mDl = NULL;

static void* getSymbol(void* dl, const char* signature) {
    void* res = dlsym(dl, signature);
    assert (res != NULL);

    return res;
}

typedef void (*InitFunc)(JpegStub* stub, int* strides);
typedef void (*CleanupFunc)(JpegStub* stub);
typedef int (*CompressFunc)(JpegStub* stub, const void* image,
        int width, int height, int quality);
typedef void (*GetCompressedImageFunc)(JpegStub* stub, void* buff);
typedef size_t (*GetCompressedSizeFunc)(JpegStub* stub);

NV21JpegCompressor::NV21JpegCompressor()
{
    const char dlName[] = "/system/lib/hw/camera.goldfish.jpeg.so";
    if (mDl == NULL) {
        mDl = dlopen(dlName, RTLD_NOW);
    }
    assert(mDl != NULL);

    InitFunc f = (InitFunc)getSymbol(mDl, "JpegStub_init");
    (*f)(&mStub, mStrides);
}

NV21JpegCompressor::~NV21JpegCompressor()
{
    CleanupFunc f = (CleanupFunc)getSymbol(mDl, "JpegStub_cleanup");
    (*f)(&mStub);
}

/****************************************************************************
 * Public API
 ***************************************************************************/

status_t NV21JpegCompressor::compressRawImage(const void* image,
                                              int width,
                                              int height,
                                              int quality)
{
    mStrides[0] = width;
    mStrides[1] = width;
    CompressFunc f = (CompressFunc)getSymbol(mDl, "JpegStub_compress");
    return (status_t)(*f)(&mStub, image, width, height, quality);
}


size_t NV21JpegCompressor::getCompressedSize()
{
    GetCompressedSizeFunc f = (GetCompressedSizeFunc)getSymbol(mDl,
            "JpegStub_getCompressedSize");
    return (*f)(&mStub);
}

void NV21JpegCompressor::getCompressedImage(void* buff)
{
    GetCompressedImageFunc f = (GetCompressedImageFunc)getSymbol(mDl,
            "JpegStub_getCompressedImage");
    (*f)(&mStub, buff);
}

}; /* namespace android */
