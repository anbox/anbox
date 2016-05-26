// Copyright (C) 2014 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "emugl/common/pod_vector.h"

#include <stdlib.h>
#include <string.h>

#define USE_MALLOC_USABLE_SIZE 0

namespace emugl {

static inline void swapPointers(char** p1, char** p2) {
    char* tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
}

PodVectorBase::PodVectorBase(const PodVectorBase& other) {
    initFrom(other.begin(), other.byteSize());
}

PodVectorBase& PodVectorBase::operator=(const PodVectorBase& other) {
    initFrom(other.begin(), other.byteSize());
    return *this;
}

PodVectorBase::~PodVectorBase() {
    if (mBegin) {
        // Sanity.
        ::memset(mBegin, 0xee, byteSize());
        ::free(mBegin);
        mBegin = NULL;
        mEnd = NULL;
        mLimit = NULL;
    }
}

void PodVectorBase::initFrom(const void* from, size_t fromLen) {
    if (!fromLen || !from) {
        mBegin = NULL;
        mEnd = NULL;
        mLimit = NULL;
    } else {
        mBegin = static_cast<char*>(::malloc(fromLen));
        mEnd = mLimit = mBegin + fromLen;
        ::memcpy(mBegin, from, fromLen);
    }
}

void PodVectorBase::assignFrom(const PodVectorBase& other) {
    resize(other.byteSize(), 1U);
    ::memmove(begin(), other.begin(), byteSize());
}

void PodVectorBase::resize(size_t newSize, size_t itemSize) {
    const size_t kMaxSize = maxItemCapacity(itemSize);
    size_t oldCapacity = itemCapacity(itemSize);
    const size_t kMinCapacity = 256 / itemSize;

    if (newSize < oldCapacity) {
        // Only shrink if the new size is really small.
        if (newSize < oldCapacity / 2 && oldCapacity > kMinCapacity) {
            reserve(newSize, itemSize);
        }
    } else if (newSize > oldCapacity) {
        size_t newCapacity = oldCapacity;
        while (newCapacity < newSize) {
            size_t newCapacity2 = newCapacity + (newCapacity >> 2) + 8;
            if (newCapacity2 < newCapacity || newCapacity > kMaxSize) {
                newCapacity = kMaxSize;
            } else {
                newCapacity = newCapacity2;
            }
        }
        reserve(newCapacity, itemSize);
    }
    mEnd = mBegin + newSize * itemSize;
}

void PodVectorBase::reserve(size_t newSize, size_t itemSize) {
    if (newSize == 0) {
        ::free(mBegin);
        mBegin = NULL;
        mEnd = NULL;
        mLimit = NULL;
        return;
    }

    size_t oldByteSize = byteSize();
    size_t newByteCapacity = newSize * itemSize;
    char* newBegin = static_cast<char*>(::realloc(mBegin, newByteCapacity));
    mBegin = newBegin;
    mEnd = newBegin + oldByteSize;
#if USE_MALLOC_USABLE_SIZE
    size_t usableSize = malloc_usable_size(mBegin);
    if (usableSize > newByteCapacity) {
        newByteCapacity = usableSize - (usableSize % itemSize);
    }
#endif
    mLimit = newBegin + newByteCapacity;
    // Sanity.
    if (newByteCapacity > oldByteSize) {
        ::memset(mBegin + oldByteSize, 0, newByteCapacity - oldByteSize);
    }
}

void PodVectorBase::removeAt(size_t itemPos, size_t itemSize) {
    size_t count = itemCount(itemSize);
    if (itemPos < count) {
        size_t  pos = itemPos * itemSize;
        ::memmove(mBegin + pos,
                  mBegin + pos + itemSize,
                  byteSize() - pos - itemSize);
        resize(count - 1U, itemSize);
    }
}

void* PodVectorBase::insertAt(size_t itemPos, size_t itemSize) {
    size_t count = this->itemCount(itemSize);
    resize(count + 1, itemSize);
    size_t pos = itemPos * itemSize;
    if (itemPos < count) {
        ::memmove(mBegin + pos + itemSize,
                  mBegin + pos,
                  count * itemSize - pos);
        // Sanity to avoid copying pointers and other bad stuff.
        ::memset(mBegin + pos, 0, itemSize);
    }
    return mBegin + pos;
}

void PodVectorBase::swapAll(PodVectorBase* other) {
    swapPointers(&mBegin, &other->mBegin);
    swapPointers(&mEnd, &other->mEnd);
    swapPointers(&mLimit, &other->mLimit);
}

}  // namespace emugl
