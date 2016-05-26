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
#pragma once

#include <vector>
#include <string>

#include <assert.h>
#include <string.h>

// Convenient class used to hold the common context data shared
// by both the GLESv1 and GLESv2 decoders. This corresponds to
// vertex attribute buffers.
class  GLDecoderContextData {
public:
    // List of supported vertex attribute indices, as they appear in
    // a glVertexAttribPointer() call.
    typedef enum  {
        VERTEX_LOCATION = 0,
        NORMAL_LOCATION = 1,
        COLOR_LOCATION = 2,
        POINTSIZE_LOCATION = 3,
        TEXCOORD0_LOCATION = 4,
        TEXCOORD1_LOCATION = 5,
        TEXCOORD2_LOCATION = 6,
        TEXCOORD3_LOCATION = 7,
        TEXCOORD4_LOCATION = 8,
        TEXCOORD5_LOCATION = 9,
        TEXCOORD6_LOCATION = 10,
        TEXCOORD7_LOCATION = 11,
        MATRIXINDEX_LOCATION = 12,
        WEIGHT_LOCATION = 13,
        LAST_LOCATION = 14
    } PointerDataLocation;

    // Default constructor.
    GLDecoderContextData(int numLocations = kMaxVertexAttributes)
            : mPointerData(),
              mNumLocations(static_cast<unsigned>(numLocations)) {
        mPointerData.resize(mNumLocations);
    }

    // Store |len| bytes from |data| into the buffer associated with
    // vertex attribute index |loc|.
    void storePointerData(unsigned int loc, void *data, size_t len) {
        if (loc < mNumLocations) {
            std::string& ptrData = mPointerData[loc];
            ptrData.assign(reinterpret_cast<char*>(data), len);
        } else {
            // User error, don't do anything here
        }
    }

    // Return pointer to data associated with vertex attribute index |loc|
    void* pointerData(unsigned int loc) const {
        if (loc < mNumLocations) {
            return const_cast<char*>(mPointerData[loc].c_str());
        } else {
            // User error. Return nullptr.
            return nullptr;
        }
    }

private:
    static const int kMaxVertexAttributes = 64;

    std::vector<std::string> mPointerData;
    unsigned mNumLocations = 0;
};
