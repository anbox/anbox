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

#include <gtest/gtest.h>

namespace emugl {

namespace {

typedef IdToObjectMapBase::KeyType KeyType;

class Foo {
public:
    Foo() : mVal(0) {}
    Foo(int val) : mVal(val) {}
    ~Foo() {}
    int val() const { return mVal; }
    void setVal(int val) { mVal = val; }
private:
    int mVal;
};

}  // namespace

TEST(IdToObjectMap, Empty) {
    IdToObjectMap<Foo> map;
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(0U, map.size());
}

TEST(IdToObjectMap, SetIntegerRange) {
    IdToObjectMap<Foo> map;
    KeyType kMax = 10000;

    // Add all items in the map.
    for (KeyType n = 0; n < kMax; ++n) {
        EXPECT_FALSE(map.set(n, new Foo(n))) << "For key " << n;
    }

    // Check final size.
    EXPECT_EQ(static_cast<size_t>(kMax), map.size());

    // Find all items in the map.
    for (KeyType n = 0; n < kMax; ++n) {
        EXPECT_TRUE(map.contains(n)) << "For key " << n;
        Foo* foo = NULL;
        EXPECT_TRUE(map.find(n, &foo)) << "For key " << n;
        if (foo) {
            EXPECT_EQ(static_cast<int>(n), foo->val()) << "For key " << n;
        }
    }
}

TEST(IdToObjectMap, RemoveAll) {
    IdToObjectMap<Foo> map;
    KeyType kMax = 10000;

    // Add all items in the map.
    for (KeyType n = 0; n < kMax; ++n) {
        EXPECT_FALSE(map.set(n, new Foo(n))) << "For key " << n;
    }

    EXPECT_EQ(static_cast<size_t>(kMax), map.size());

    for (KeyType n = 0; n < kMax; ++n) {
        EXPECT_TRUE(map.remove(n)) << "For key " << n;
    }
    EXPECT_EQ(0U, map.size());
}

TEST(IdToObjectMap, RemoveOdd) {
    IdToObjectMap<Foo> map;
    KeyType kMax = 10000;

    // Add all items in the map.
    for (KeyType n = 0; n < kMax; ++n) {
        EXPECT_FALSE(map.set(n, new Foo(n))) << "For key " << n;
    }

    EXPECT_EQ(static_cast<size_t>(kMax), map.size());

    for (KeyType n = 0; n < kMax; ++n) {
        if (n & 1) {
            EXPECT_TRUE(map.remove(n)) << "For key " << n;
        }
    }
    EXPECT_EQ(static_cast<size_t>(kMax / 2), map.size());

    for (KeyType n = 0; n < kMax; ++n) {
        if (n & 1) {
            EXPECT_FALSE(map.contains(n)) << "For key " << n;
        } else {
            EXPECT_TRUE(map.contains(n)) << "For key " << n;
            Foo* foo = NULL;
            EXPECT_TRUE(map.find(n, &foo)) << "For key " << n;
            if (foo) {
                EXPECT_EQ(static_cast<int>(n), foo->val());
            }
        }
    }
}

}  // namespace emugl
