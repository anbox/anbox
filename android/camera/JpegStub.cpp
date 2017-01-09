/*
 * Copyright (C) 2013 The Android Open Source Project
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

#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera_JPEGStub"
#include <errno.h>
#include <cutils/log.h>
#include <YuvToJpegEncoder.h>

#include "JpegStub.h"


extern "C" void JpegStub_init(JpegStub* stub, int* strides) {
    stub->mInternalEncoder = (void*) new Yuv420SpToJpegEncoder(strides);
    stub->mInternalStream = (void*)new SkDynamicMemoryWStream();
}

extern "C" void JpegStub_cleanup(JpegStub* stub) {
    delete((Yuv420SpToJpegEncoder*)stub->mInternalEncoder);
    delete((SkDynamicMemoryWStream*)stub->mInternalStream);
}

extern "C" int JpegStub_compress(JpegStub* stub, const void* image,
        int width, int height, int quality)
{
    void* pY = const_cast<void*>(image);
    int offsets[2];
    offsets[0] = 0;
    offsets[1] = width * height;

    Yuv420SpToJpegEncoder* encoder =
        (Yuv420SpToJpegEncoder*)stub->mInternalEncoder;
    SkDynamicMemoryWStream* stream =
        (SkDynamicMemoryWStream*)stub->mInternalStream;
    if (encoder->encode(stream, pY, width, height, offsets, quality)) {
        ALOGV("%s: Compressed JPEG: %d[%dx%d] -> %zu bytes",
              __FUNCTION__, (width * height * 12) / 8,
              width, height, stream->getOffset());
        return 0;
    } else {
        ALOGE("%s: JPEG compression failed", __FUNCTION__);
        return errno ? errno: EINVAL;
    }
}

extern "C" void JpegStub_getCompressedImage(JpegStub* stub, void* buff) {
    SkDynamicMemoryWStream* stream =
        (SkDynamicMemoryWStream*)stub->mInternalStream;
    stream->copyTo(buff);
}

extern "C" size_t JpegStub_getCompressedSize(JpegStub* stub) {
    SkDynamicMemoryWStream* stream =
        (SkDynamicMemoryWStream*)stub->mInternalStream;
    return stream->getOffset();
}
