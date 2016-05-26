// Copyright 2014 The Android Open Source Project
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

#include "emugl/common/message_channel.h"

#include "emugl/common/testing/test_thread.h"

#include <gtest/gtest.h>

#include <string>

namespace emugl {

namespace {

struct PingPongState {
    MessageChannel<std::string, 3U> in;
    MessageChannel<std::string, 3U> out;
};

void* pingPongFunction(void* param) {
    for (;;) {
        PingPongState* s = static_cast<PingPongState*>(param);
        std::string str;
        s->in.receive(&str);
        s->out.send(str);
        if (str == "quit") {
            break;
        }
    }
    return 0;
}

}  // namespace

TEST(MessageChannel, SingleThreadWithInt) {
    MessageChannel<int, 3U> channel;
    channel.send(1);
    channel.send(2);
    channel.send(3);

    int ret;
    channel.receive(&ret);
    EXPECT_EQ(1, ret);
    channel.receive(&ret);
    EXPECT_EQ(2, ret);
    channel.receive(&ret);
    EXPECT_EQ(3, ret);
}

TEST(MessageChannel, SingleThreadWithStdString) {
    MessageChannel<std::string, 5U> channel;
    channel.send(std::string("foo"));
    channel.send(std::string("bar"));
    channel.send(std::string("zoo"));

    std::string str;
    channel.receive(&str);
    EXPECT_STREQ("foo", str.c_str());
    channel.receive(&str);
    EXPECT_STREQ("bar", str.c_str());
    channel.receive(&str);
    EXPECT_STREQ("zoo", str.c_str());
}

TEST(MessageChannel, TwoThreadsPingPong) {
    PingPongState state;
    TestThread* thread = new TestThread(pingPongFunction, &state);

    std::string str;
    const size_t kCount = 100;
    for (size_t n = 0; n < kCount; ++n) {
        state.in.send(std::string("foo"));
        state.in.send(std::string("bar"));
        state.in.send(std::string("zoo"));
        state.out.receive(&str);
        EXPECT_STREQ("foo", str.c_str());
        state.out.receive(&str);
        EXPECT_STREQ("bar", str.c_str());
        state.out.receive(&str);
        EXPECT_STREQ("zoo", str.c_str());
    }
    state.in.send(std::string("quit"));
    state.out.receive(&str);
    EXPECT_STREQ("quit", str.c_str());

    thread->join();
}

}  // namespace emugl
