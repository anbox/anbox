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

#include <gtest/gtest.h>

namespace emugl {

static int  hashIndex(size_t n) {
    return static_cast<int>(((n >> 14) * 13773) + (n * 51));
}

TEST(PodVector, Empty) {
    PodVector<int> v;
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(0U, v.size());
}

TEST(PodVector, AppendOneItem) {
    PodVector<int> v;
    v.append(10234);
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(1U, v.size());
    EXPECT_EQ(10234, v[0]);
}

TEST(PodVector, AppendLotsOfItems) {
    PodVector<int> v;
    const size_t kMaxCount = 10000;
    for (size_t n = 0; n < kMaxCount; ++n) {
        v.append(hashIndex(n));
    }
    EXPECT_EQ(kMaxCount, v.size());
    for (size_t n = 0; n < kMaxCount; ++n) {
        EXPECT_EQ(hashIndex(n), v[n]) << "At index " << n;
    }
}

TEST(PodVector, RemoveFrontItems) {
    PodVector<int> v;
    const size_t kMaxCount = 100;
    for (size_t n = 0; n < kMaxCount; ++n) {
        v.append(hashIndex(n));
    }
    EXPECT_EQ(kMaxCount, v.size());
    for (size_t n = 0; n < kMaxCount; ++n) {
        EXPECT_EQ(hashIndex(n), v[0]) << "At index " << n;
        v.remove(0U);
        EXPECT_EQ(kMaxCount - n - 1U, v.size()) << "At index " << n;
    }
}

TEST(PodVector, PrependItems) {
    PodVector<int> v;
    const size_t kMaxCount = 100;
    for (size_t n = 0; n < kMaxCount; ++n) {
        v.prepend(hashIndex(n));
    }
    EXPECT_EQ(kMaxCount, v.size());
    for (size_t n = 0; n < kMaxCount; ++n) {
        EXPECT_EQ(hashIndex(kMaxCount - n - 1), v[n]) << "At index " << n;
    }
}

TEST(PodVector, ResizeExpands) {
    PodVector<int> v;
    const size_t kMaxCount = 100;
    const size_t kMaxCount2 = 10000;
    for (size_t n = 0; n < kMaxCount; ++n) {
        v.append(hashIndex(n));
    }
    EXPECT_EQ(kMaxCount, v.size());
    v.resize(kMaxCount2);
    EXPECT_EQ(kMaxCount2, v.size());
    for (size_t n = 0; n < kMaxCount; ++n) {
        EXPECT_EQ(hashIndex(n), v[n]) << "At index " << n;
    }
}

TEST(PodVector, ResizeTruncates) {
    PodVector<int> v;
    const size_t kMaxCount = 10000;
    const size_t kMaxCount2 = 10;
    for (size_t n = 0; n < kMaxCount; ++n) {
        v.append(hashIndex(n));
    }
    EXPECT_EQ(kMaxCount, v.size());
    v.resize(kMaxCount2);
    EXPECT_EQ(kMaxCount2, v.size());
    for (size_t n = 0; n < kMaxCount2; ++n) {
        EXPECT_EQ(hashIndex(n), v[n]) << "At index " << n;
    }
}


TEST(PodVector, AssignmentOperator) {
    PodVector<int> v1;
    const size_t kMaxCount = 10000;
    for (size_t n = 0; n < kMaxCount; ++n) {
        v1.append(hashIndex(n));
    }
    EXPECT_EQ(kMaxCount, v1.size());

    PodVector<int> v2;
    v2 = v1;

    v1.reserve(0);

    EXPECT_EQ(kMaxCount, v2.size());
    for (size_t n = 0; n < kMaxCount; ++n) {
        EXPECT_EQ(hashIndex(n), v2[n]) << "At index " << n;
    }
}

}  // namespace emugl
