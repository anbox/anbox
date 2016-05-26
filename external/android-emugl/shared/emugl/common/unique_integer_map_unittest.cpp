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

#include "emugl/common/unique_integer_map.h"

#include <gtest/gtest.h>

#include <stdio.h>

namespace emugl {

typedef UniqueIntegerMap<uintptr_t,uint32_t> MyMap;

TEST(UniqueIntegerMap, Empty) {
    MyMap map;

    EXPECT_TRUE(map.empty());
    EXPECT_EQ(0U, map.size());
    EXPECT_EQ(0U, map.find(0U));
    EXPECT_EQ(0U, map.find(1U));
    EXPECT_EQ(0U, map.find(2U));
    EXPECT_EQ(0U, map.find(4U));
}

TEST(UniqueIntegerMap, AddOne) {
    MyMap map;
    uintptr_t key1 = 1U;
    uint32_t val1 = map.add(key1);

    EXPECT_NE(0U, val1);
    EXPECT_EQ(val1, map.find(key1));
    EXPECT_EQ(key1, map.findKeyFor(val1));

    EXPECT_FALSE(map.empty());
    EXPECT_EQ(1U, map.size());

    EXPECT_EQ(0U, map.find(0));
    EXPECT_EQ(0U, map.findKeyFor(0));

    EXPECT_EQ(0U, map.find(key1 + 1));
    EXPECT_EQ(0U, map.findKeyFor(val1 + 1));
}

TEST(UniqueIntegerMap, AddMultiple) {
    MyMap map;
    const size_t kCount = 100;
    const size_t kKeyMultiplier = 3U;  // must be >= 2.
    uint32_t values[kCount];

    for (size_t n = 0; n < kCount; ++n) {
        uintptr_t key = 1U + n * kKeyMultiplier;
        values[n] = map.add(key);
        EXPECT_NE(0U, values[n]) << "key #" << n;
    }

    EXPECT_EQ(kCount, map.size());

    for (size_t n = 0; n < kCount; ++n) {
        uintptr_t key = 1U + n * kKeyMultiplier;
        EXPECT_EQ(values[n], map.find(key)) << "key #" << n;
        EXPECT_EQ(0U, map.find(key + 1U)) << "key #" << n;
    }

    for (size_t n = 0; n < kCount; ++n) {
        uintptr_t key = 1U + n * kKeyMultiplier;
        EXPECT_EQ(key, map.findKeyFor(values[n]));
    }
}

TEST(UniqueIntegerMap, Del) {
    MyMap map;
    const size_t kCount = 100;
    const size_t kKeyMultiplier = 3U;  // must be >= 2.
    uint32_t values[kCount];

    for (size_t n = 0; n < kCount; ++n) {
        uintptr_t key = 1U + n * kKeyMultiplier;
        values[n] = map.add(key);
    }

    for (size_t n = 0; n < kCount; ++n) {
        uintptr_t key = 1U + n * kKeyMultiplier;
        map.del(key);
        EXPECT_EQ(kCount - 1U - n, map.size());
        EXPECT_EQ(0U, map.find(key));
        EXPECT_EQ(0U, map.findKeyFor(values[n]));
    }

    EXPECT_TRUE(map.empty());
}

}  // namespace emugl
