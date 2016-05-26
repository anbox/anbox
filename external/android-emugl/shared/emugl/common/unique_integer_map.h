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

#ifndef EMUGL_COMMON_UNIQUE_INTEGER_MAP_H
#define EMUGL_COMMON_UNIQUE_INTEGER_MAP_H

#include "emugl/common/pod_vector.h"

#include <stdint.h>

namespace emugl {

// Helper template class that implements a bi-directional mapping between
// two integer types |A| and |B|. More specifically:
//
// - The map allocates values of type |B| when a key of type |A| is entered
//   in the map.
//
// - keys and values cannot be 0, which is reserved (i.e. means 'invalid').
//
// This is used in EmuGL to map liberal 'void*' values (e.g. EGLimages ones)
// to unique 32-bit IDs that can be written to / read from the wire protocol.
template <typename A, typename B>
class UniqueIntegerMap {
public:
    UniqueIntegerMap() : mForwardPairs(), mBackwardPairs() {}
    ~UniqueIntegerMap() {}

    // Return true iff the map is empty.
    const bool empty() const { return mForwardPairs.empty(); }

    // Return the number of (key,value) pairs in the map.
    size_t size() const { return mForwardPairs.size(); }

    // Find the value associated with |key| in the map.
    // Returns 0 in case of failure, or if |key| is 0.
    B find(const A key) const;

    // Find the key associated with a given |value| in the map.
    // Returns 0 if |value| is 0, or in case of failure.
    A findKeyFor(const B value) const;

    // Add |key| to the map and return an automatically-allocated
    // unique value for it. Return 0 if |key| is 0.
    B add(const A key);

    // Delete the entry associated with a given |key|. The
    // corresponding value may be recycled by future calls to add().
    void del(const A key);

private:
    typedef struct {
        A first;
        B second;
    } ForwardPair;

    typedef struct {
        B first;
        A second;
    } BackwardPair;

    size_t findKeyIndexPlusOne(const A key) const;
    size_t findValueIndexPlusOne(const B value) const;

    B allocValue();
    void freeValue(B value);

    PodVector<ForwardPair> mForwardPairs;
    PodVector<BackwardPair> mBackwardPairs;

    B mLastValue;
    PodVector<B> mFreeValues;
};

template <typename A, typename B>
B UniqueIntegerMap<A,B>::find(const A key) const {
    size_t keyIndex = findKeyIndexPlusOne(key);
    if (!keyIndex) {
        return 0;
    }
    return mForwardPairs[keyIndex - 1U].second;
}

template <typename A, typename B>
A UniqueIntegerMap<A,B>::findKeyFor(const B value) const {
    size_t valueIndex = findValueIndexPlusOne(value);
    if (!valueIndex) {
        return 0;
    }
    return mBackwardPairs[valueIndex - 1U].second;
}

template <typename A, typename B>
B UniqueIntegerMap<A,B>::add(const A key) {
    // Binary search to find the proper insertion point for the key.
    // Also checks that the key isn't already in the set.
    size_t min = 0;
    size_t max = mForwardPairs.size();
    while (min < max) {
        size_t mid = min + ((max - min) >> 1);
        A midKey = mForwardPairs[mid].first;
        if (midKey < key) {
            min = mid + 1U;
        } else if (midKey > key) {
            max = mid;
        } else {
            // Already in the set.
            return 0;
        }
    }

    // Generate new unique value
    B value = allocValue();

    ForwardPair* pair = mForwardPairs.emplace(min);
    pair->first = key;
    pair->second = value;

    // Binary search to find proper insertion point for the value.
    min = 0;
    max = mBackwardPairs.size();
    while (min < max) {
        size_t mid = min + ((max - min) >> 1);
        B midValue = mBackwardPairs[mid].first;
        if (midValue < value) {
            min = mid + 1U;
        } else {
            max = mid;
        }
    }

    BackwardPair* backPair = mBackwardPairs.emplace(min);
    backPair->first = value;
    backPair->second = key;

    return value;
}

template <typename A, typename B>
void UniqueIntegerMap<A,B>::del(const A key) {
    size_t keyIndex = findKeyIndexPlusOne(key);
    if (!keyIndex) {
        return;
    }
    B value = mForwardPairs[keyIndex - 1U].second;
    size_t valueIndex = findValueIndexPlusOne(value);
    mForwardPairs.remove(keyIndex - 1U);
    mBackwardPairs.remove(valueIndex - 1U);
    freeValue(value);
}

template <typename A, typename B>
size_t UniqueIntegerMap<A,B>::findKeyIndexPlusOne(const A key) const {
    // Binary search in forward pair array.
    size_t min = 0;
    size_t max = mForwardPairs.size();
    while (min < max) {
        size_t mid = min + ((max - min) >> 1);
        A midKey = mForwardPairs[mid].first;
        if (midKey < key) {
            min = mid + 1U;
        } else if (midKey > key) {
            max = mid;
        } else {
            return mid + 1U;
        }
    }
    return 0U;
}

template <typename A, typename B>
size_t UniqueIntegerMap<A,B>::findValueIndexPlusOne(const B value) const {
    // Binary search in revere pair array.
    size_t min = 0;
    size_t max = mBackwardPairs.size();
    while (min < max) {
        size_t mid = min + ((max - min) >> 1);
        B midValue = mBackwardPairs[mid].first;
        if (midValue < value) {
            min = mid + 1U;
        } else if (midValue > value) {
            max = mid;
        } else {
            return mid + 1U;
        }
    }
    return 0U;
}

template <typename A, typename B>
B UniqueIntegerMap<A,B>::allocValue() {
    if (!mFreeValues.empty()) {
        B result = mFreeValues[0];
        mFreeValues.pop();
        return result;
    }
    return ++mLastValue;
}

template <typename A, typename B>
void UniqueIntegerMap<A,B>::freeValue(B value) {
    if (!value) {
        return;
    }
    if (value == mLastValue) {
        mLastValue--;
        return;
    }
    mFreeValues.append(value);
}

}  // namespace emugl

#endif  // EMUGL_COMMON_INTEGER_MAP_H
