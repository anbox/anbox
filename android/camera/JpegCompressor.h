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

#ifndef HW_EMULATOR_CAMERA_JPEG_COMPRESSOR_H
#define HW_EMULATOR_CAMERA_JPEG_COMPRESSOR_H

/*
 * Contains declaration of a class NV21JpegCompressor that encapsulates a
 * converter between YV21, and JPEG formats.
 */

#include "JpegStub.h"
#include <utils/threads.h>

namespace android {

/* Encapsulates a converter between YV12, and JPEG formats.
 */
class NV21JpegCompressor
{
public:
    /* Constructs JpegCompressor instance. */
    NV21JpegCompressor();
    /* Destructs JpegCompressor instance. */
    ~NV21JpegCompressor();

    /****************************************************************************
     * Public API
     ***************************************************************************/

public:
    /* Compresses raw NV21 image into a JPEG.
     * The compressed image will be saved in mStream member of this class. Use
     * getCompressedSize method to obtain buffer size of the compressed image,
     * and getCompressedImage to copy out the compressed image.
     * Param:
     *  image - Raw NV21 image.
     *  width, height - Image dimensions.
     *  quality - JPEG quality.
     * Return:
     *  NO_ERROR on success, or an appropriate error status.
     *
     */
    status_t compressRawImage(const void* image,
                              int width,
                              int height,
                              int quality);

    /* Get size of the compressed JPEG buffer.
     * This method must be called only after a successful completion of
     * compressRawImage call.
     * Return:
     *  Size of the compressed JPEG buffer.
     */
    size_t getCompressedSize();

    /* Copies out compressed JPEG buffer.
     * This method must be called only after a successful completion of
     * compressRawImage call.
     * Param:
     *  buff - Buffer where to copy the JPEG. Must be large enough to contain the
     *      entire image.
     */
    void getCompressedImage(void* buff);

    /****************************************************************************
     * Class data
     ***************************************************************************/

protected:
    /* Strides for Y (the first element), and UV (the second one) panes. */
    int                     mStrides[2];

private:
    // library handle to dlopen
    static void* mDl;
    JpegStub mStub;
};

}; /* namespace android */

#endif  /* HW_EMULATOR_CAMERA_JPEG_COMPRESSOR_H */
