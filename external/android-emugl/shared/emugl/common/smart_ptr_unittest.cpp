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

#include "emugl/common/smart_ptr.h"

#include <gtest/gtest.h>

namespace emugl {

// This Test sub-class is used to track allocations and deallocations of
// the MyTestClass instances that are created through newInstance().
// See below for typical usage.
class SmartPtrTest : public testing::Test {
public:
    SmartPtrTest() : mNewCount(0), mDeleteCount(0), mDoCount(0) {}

    ~SmartPtrTest() {
        mNewCount = 0;
        mDoCount = 0;
        mDeleteCount = 0;
    }

    class MyClass;

    MyClass* newInstance() {
        return new MyClass(this);
    }

    class MyClass {
    public:
        MyClass(SmartPtrTest* test) : mTest(test) {
            mTest->mNewCount++;
        }

        void doStuff() {
            mTest->mDoCount++;
        }

        ~MyClass() {
            mTest->mDeleteCount++;
        }
    private:
        SmartPtrTest* mTest;
    };

    int mNewCount;
    int mDeleteCount;
    int mDoCount;
};


TEST_F(SmartPtrTest, Empty) {
    SmartPtr<MyClass> ptr;
    EXPECT_FALSE(ptr.Ptr());

    EXPECT_EQ(0, mNewCount);
    EXPECT_EQ(0, mDeleteCount);
    EXPECT_EQ(0, mDoCount);
}


TEST_F(SmartPtrTest, SingleRef) {
    MyClass* obj = newInstance();
    EXPECT_EQ(1, mNewCount);

    {
        SmartPtr<MyClass> ptr(obj);
        EXPECT_EQ(obj, ptr.Ptr());

        EXPECT_EQ(1, mNewCount);
        EXPECT_EQ(0, mDeleteCount);
        EXPECT_EQ(0, mDoCount);
    }
    // Check that the object was deleted.
    EXPECT_EQ(1, mDeleteCount);
}


TEST_F(SmartPtrTest, CopyConstructor) {
    MyClass* obj = newInstance();
    EXPECT_EQ(1, mNewCount);

    {
        SmartPtr<MyClass> ptr1(obj);
        {
            SmartPtr<MyClass> ptr2(ptr1);
            EXPECT_EQ(2, ptr1.getRefCount());
            EXPECT_EQ(2, ptr2.getRefCount());
            EXPECT_EQ(1, mNewCount);
            EXPECT_EQ(0, mDeleteCount);
            EXPECT_EQ(0, mDoCount);
        }
        EXPECT_EQ(1, mNewCount);
        EXPECT_EQ(0, mDeleteCount);
        EXPECT_EQ(0, mDoCount);
    }
    EXPECT_EQ(1, mNewCount);
    EXPECT_EQ(1, mDeleteCount);
    EXPECT_EQ(0, mDoCount);
}


TEST_F(SmartPtrTest, AssignmentOperator) {
    SmartPtr<MyClass> ptr1(newInstance());
    SmartPtr<MyClass> ptr2(newInstance());
    EXPECT_EQ(2, mNewCount);
    EXPECT_EQ(0, mDeleteCount);
    EXPECT_EQ(0, mDoCount);

    ptr2 = ptr1;
    EXPECT_EQ(2, mNewCount);
    EXPECT_EQ(1, mDeleteCount);

    EXPECT_EQ(ptr1.Ptr(), ptr2.Ptr());
    EXPECT_EQ(2, ptr1.getRefCount());
    EXPECT_EQ(2, ptr2.getRefCount());
}


TEST_F(SmartPtrTest, ArrowOperator) {
    SmartPtr<MyClass> ptr(newInstance());
    ptr->doStuff();
    EXPECT_EQ(1, mDoCount);

    (*ptr).doStuff();
    EXPECT_EQ(2, mDoCount);
}

}  // namespace emugl
