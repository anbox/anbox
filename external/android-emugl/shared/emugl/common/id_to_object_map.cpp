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

#include "emugl/common/id_to_object_map.h"

#include <stdlib.h>

namespace emugl {

namespace {

typedef IdToObjectMapBase::KeyType KeyType;

enum {
    kMinShift = 3,
    kMaxShift = 31,
    kMinCapacity = (1 << kMinShift),
    kLoadScale = 1024,
    kMinLoad = kLoadScale/4,    // 25% minimum load.
    kMaxLoad = kLoadScale*3/4,  // 75% maximum load.

    kInvalidKey = IdToObjectMapBase::kMaxId + 1U,
    kTombstone = IdToObjectMapBase::kMaxId + 2U,
};

// Return a number that indicates if the current |capacity| is appropriate
// to hold |size| items in our map.
// -1 -> the capacity is too small and needs to be increased.
//  0 -> the capacity is ok.
// +1 -> the capacity is too large and needs to be decreased.
int capacityCompare(size_t shift, size_t size) {
    size_t capacity = 1U << shift;
    // Essentially, one can rewrite:
    //   load < minLoad
    // as:
    //   size / capacity < minLoad
    //   capacity * minLoad > size
    if (capacity * kMinLoad  > size * kLoadScale)
        return +1;

    // Similarly, one can rewrite:
    //   load > maxLoad
    // as:
    //   size / capacity > maxLoad
    //   capacity * maxLoad < size
    if (capacity * kMaxLoad < size * kLoadScale)
        return -1;

    return 0;
}

size_t probeKeys(const KeyType* keys, size_t shift, KeyType key) {
    static const int kPrimes[] = {
    1,          /* For 1 << 0 */
    2,
    3,
    7,
    13,
    31,
    61,
    127,
    251,
    509,
    1021,
    2039,
    4093,
    8191,
    16381,
    32749,
    65521,      /* For 1 << 16 */
    131071,
    262139,
    524287,
    1048573,
    2097143,
    4194301,
    8388593,
    16777213,
    33554393,
    67108859,
    134217689,
    268435399,
    536870909,
    1073741789,
    2147483647  /* For 1 << 31 */
    };

    size_t slot = key % kPrimes[shift];
    size_t step = 0;
    for (;;) {
        KeyType k = keys[slot];
        if (k == kInvalidKey || k == kTombstone || k == key)
            return slot;

        step += 1;
        slot = (slot + step) & (1U << shift);
    }
}

}  // namespace

IdToObjectMapBase::IdToObjectMapBase() :
        mCount(0), mShift(kMinShift) {
    size_t capacity = 1U << mShift;
    mKeys = static_cast<KeyType*>(::calloc(sizeof(mKeys[0]), capacity));
    mValues = static_cast<void**>(::calloc(sizeof(mValues[0]), capacity));
    for (size_t n = 0; n < capacity; ++n) {
        mKeys[n] = kInvalidKey;
    }
}

IdToObjectMapBase::~IdToObjectMapBase() {
    mShift = 0;
    mCount = 0;
    ::free(mKeys);
    ::free(mValues);
}

bool IdToObjectMapBase::contains(KeyType key) const {
    size_t slot = probeKeys(mKeys, mShift, key);
    switch (mKeys[slot]) {
        case kInvalidKey:
        case kTombstone:
            return false;
        default:
            ;
    }
    return true;
}

bool IdToObjectMapBase::find(KeyType key, void** value) const {
    size_t slot = probeKeys(mKeys, mShift, key);
    if (!isValidKey(mKeys[slot])) {
        *value = NULL;
        return false;
    }
    *value = mValues[slot];
    return true;
}

void* IdToObjectMapBase::set(KeyType key, void* value) {
    if (!value)
        return remove(key);

    size_t slot = probeKeys(mKeys, mShift, key);
    void* result;
    if (isValidKey(mKeys[slot])) {
        result = mValues[slot];
        mValues[slot] = value;
    } else {
        mKeys[slot] = key;
        mValues[slot] = value;
        result = NULL;
        mCount++;
        resize(mCount);
    }
    return result;
}

void* IdToObjectMapBase::remove(KeyType key) {
    size_t slot = probeKeys(mKeys, mShift, key);
    if (!isValidKey(mKeys[slot]))
        return NULL;

    void* result = mValues[slot];
    mValues[slot] = NULL;
    mKeys[slot] = kTombstone;
    mCount--;
    return result;
}

void IdToObjectMapBase::resize(size_t newSize) {
    int ret = capacityCompare(mShift, newSize);
    if (!ret)
        return;

    size_t oldCapacity = 1U << mShift;
    size_t newShift = mShift;

    if (ret < 0) {
        // Capacity is too small and must be increased.
        do {
            if (newShift == kMaxShift)
                break;
            ++newShift;
        } while (capacityCompare(newShift, newSize) < 0);
    } else {
        // Capacity is too large and must be decreased.
        do {
            if (newShift == kMinShift)
                break;
            newShift--;
        } while (capacityCompare(newShift, newSize) > 0);
    }
    if (newShift == mShift)
        return;

    // Allocate new arrays.
    size_t newCapacity = 1U << newShift;
    KeyType* newKeys = static_cast<KeyType*>(
            ::calloc(sizeof(newKeys[0]), newCapacity));
    void** newValues = static_cast<void**>(
            ::calloc(sizeof(newValues[0]), newCapacity));
    for (size_t n = 0; n < newCapacity; ++n)
        newKeys[n] = kInvalidKey;

    // Copy old entries into new arrays.
    for (size_t n = 0; n < oldCapacity; ++n) {
        KeyType key = mKeys[n];
        if (isValidKey(key)) {
            size_t newSlot = probeKeys(newKeys, newShift, key);
            newKeys[newSlot] = key;
            newValues[newSlot] = mValues[n];
        }
    }

    // Swap arrays, and get rid of old ones.
    ::free(mKeys);
    ::free(mValues);
    mKeys = newKeys;
    mValues = newValues;
    mShift = newShift;
}

}  // namespace emugl
