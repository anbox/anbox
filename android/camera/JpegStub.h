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

#ifndef JPEGSTUB_H_
#define JPEGSTUB_H_

extern "C" {

struct JpegStub {
    void* mInternalEncoder;
    void* mInternalStream;
};

void JpegStub_init(JpegStub* stub, int* strides);
void JpegStub_cleanup(JpegStub* stub);
int JpegStub_compress(JpegStub* stub, const void* image,
        int width, int height, int quality);
void JpegStub_getCompressedImage(JpegStub* stub, void* buff);
size_t JpegStub_getCompressedSize(JpegStub* stub);

};
#endif // JPEGSTUB_H_
