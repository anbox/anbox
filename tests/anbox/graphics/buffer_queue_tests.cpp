// Copyright (C) 2016 The Android Open Source Project
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

#include "anbox/graphics/buffer_queue.h"
#include "anbox/common/message_channel.h"
#include "anbox/logger.h"

#include <gtest/gtest.h>

#include <thread>

namespace anbox {
namespace graphics {
TEST(BufferQueue, Constructor) {
    BufferQueue queue(16);
}

TEST(BufferQueue, TryPushLocked) {
    BufferQueue queue(2);

    EXPECT_EQ(0, queue.try_push_locked(Buffer("Hello")));
    EXPECT_EQ(0, queue.try_push_locked(Buffer("World")));

    Buffer buff0("You Shall Not Move");
    EXPECT_EQ(-EAGAIN, queue.try_push_locked(std::move(buff0)));
    EXPECT_FALSE(buff0.empty()) << "Buffer should not be moved on failure!";
}

TEST(BufferQueue, TryPushLockedOnClosedQueue) {
    BufferQueue queue(2);

    EXPECT_EQ(0, queue.try_push_locked(Buffer("Hello")));

    // Closing the queue prevents pushing new items to the queue.
    queue.close_locked();

    EXPECT_EQ(-EIO, queue.try_push_locked(Buffer("World")));
}

TEST(BufferQueue, TryPopLocked) {
    BufferQueue queue(2);;

    Buffer buffer;
    EXPECT_EQ(-EAGAIN, queue.try_pop_locked(&buffer));

    EXPECT_EQ(0, queue.try_push_locked(Buffer("Hello")));
    EXPECT_EQ(0, queue.try_push_locked(Buffer("World")));

    EXPECT_EQ(0, queue.try_pop_locked(&buffer));
    EXPECT_STREQ("Hello", buffer.data());

    EXPECT_EQ(0, queue.try_pop_locked(&buffer));
    EXPECT_STREQ("World", buffer.data());

    EXPECT_EQ(-EAGAIN, queue.try_pop_locked(&buffer));
    EXPECT_STREQ("World", buffer.data());
}

TEST(BufferQueue, TryPopLockedOnClosedQueue) {
    BufferQueue queue(2);

    Buffer buffer;
    EXPECT_EQ(-EAGAIN, queue.try_pop_locked(&buffer));

    EXPECT_EQ(0, queue.try_push_locked(Buffer("Hello")));
    EXPECT_EQ(0, queue.try_push_locked(Buffer("World")));

    EXPECT_EQ(0, queue.try_pop_locked(&buffer));
    EXPECT_STREQ("Hello", buffer.data());

    // Closing the queue doesn't prevent popping existing items, but
    // will generate -EIO once it is empty.
    queue.close_locked();

    EXPECT_EQ(0, queue.try_pop_locked(&buffer));
    EXPECT_STREQ("World", buffer.data());

    EXPECT_EQ(-EIO, queue.try_pop_locked(&buffer));
    EXPECT_STREQ("World", buffer.data());
}

namespace {
// A TestThread instance that holds a reference to a queue and can either
// push or pull to it, on command from another thread. This uses a
// MessageChannel to implement the communication channel between the
// command thread and this one.
class TestThread final {
 public:
  TestThread(std::mutex &lock, BufferQueue &queue) :
    lock_(lock),
    queue_(queue) {
  }

  bool start() {
    thread_ = std::thread(&TestThread::thread_main, this);
    return true;
  }

  // Tell the test thread to push |buffer| to the queue.
  // Call endPush() later to get the command's result.
  bool start_push(Buffer&& buffer) {
    input_.send(Request{Cmd::Push, std::move(buffer)});
    return true;
  }

  int end_push() {
    Reply reply = {};
    output_.receive(&reply);
    return reply.result;
  }

  // Tell the test thread to pop a buffer from the queue.
  // Call end_pop() to get the command's result, as well as the popped
  // buffer if it is 0.
  bool start_pop() {
    input_.send(Request{Cmd::Pop, Buffer{}});
    return true;
  }

  // Return the result of a previous start_pop() command. If result is
  // 0, sets |*buffer| to the result buffer.
  int end_pop(Buffer* buffer) {
    Reply reply = {};
    output_.receive(&reply);
    if (reply.result == 0)
        *buffer = std::move(reply.buffer);
    return reply.result;
  }

  // Tell the test thread to close the queue from its side.
  void do_close() {
    input_.send(Request{Cmd::Close, Buffer{}});
  }

  // Tell the test thread to stop after completing its current command.
  void stop() {
    input_.send(Request{Cmd::Stop, Buffer{}});
    thread_.join();
  }

 private:
  enum class Cmd {
      Push,
      Pop,
      Close,
      Stop,
  };

  struct Request {
      Cmd cmd;
      Buffer buffer;
  };

  struct Reply {
      int result;
      Buffer buffer;
  };

  void thread_main() {
    while (true) {
      Request r;
      input_.receive(&r);
      if (r.cmd == Cmd::Stop)
          break;
      std::unique_lock<std::mutex> l(lock_);
      Reply reply = {};
      bool sendReply = false;
      switch (r.cmd) {
          case Cmd::Push:
              reply.result = queue_.push_locked(std::move(r.buffer), l);
              sendReply = true;
              break;

          case Cmd::Pop:
              reply.result = queue_.pop_locked(&reply.buffer, l);
              sendReply = true;
              break;

          case Cmd::Close:
              queue_.close_locked();
              break;

          default:
              ;
      }
      if (sendReply)
          output_.send(std::move(reply));
    }
  }

  std::thread thread_;
  std::mutex &lock_;
  BufferQueue &queue_;
  anbox::common::MessageChannel<Request, 4> input_;
  anbox::common::MessageChannel<Reply, 4> output_;
};
} // namespace

TEST(BufferQueue, PushLocked) {
    std::mutex lock;
    BufferQueue queue(2);
    TestThread thread(lock, queue);

    ASSERT_TRUE(thread.start());
    ASSERT_TRUE(thread.start_pop());

    std::unique_lock<std::mutex> l(lock);
    EXPECT_EQ(0, queue.push_locked(Buffer("Hello"), l));
    EXPECT_EQ(0, queue.push_locked(Buffer("World"), l));
    EXPECT_EQ(0, queue.push_locked(Buffer("Foo"), l));

    thread.stop();
}

TEST(BufferQueue, PushLockedWithClosedQueue) {
    std::mutex lock;
    BufferQueue queue(2);
    TestThread thread(lock, queue);

    ASSERT_TRUE(thread.start());

    {
      std::unique_lock<std::mutex> l(lock);
      EXPECT_EQ(0, queue.push_locked(Buffer("Hello"), l));
      // Closing the queue prevents pushing new items, but not
      // pulling from the queue.
      queue.close_locked();
      EXPECT_EQ(-EIO, queue.push_locked(Buffer("World"), l));
    }

    Buffer buffer;
    ASSERT_TRUE(thread.start_pop());
    EXPECT_EQ(0, thread.end_pop(&buffer));
    EXPECT_STREQ("Hello", buffer.data());

    thread.stop();
}

TEST(BufferQueue, PopLocked) {
    std::mutex lock;
    BufferQueue queue(2);
    TestThread thread(lock, queue);

    ASSERT_TRUE(thread.start());
    ASSERT_TRUE(thread.start_push(Buffer("Hello World")));
    EXPECT_EQ(0, thread.end_push());

    {
      std::unique_lock<std::mutex> l(lock);
      Buffer buffer;
      EXPECT_EQ(0, queue.pop_locked(&buffer, l));
      EXPECT_STREQ("Hello World", buffer.data());
    }

    thread.stop();
}

TEST(BufferQueue, PopLockedWithClosedQueue) {
    std::mutex lock;
    BufferQueue queue(2);
    TestThread thread(lock, queue);

    ASSERT_TRUE(thread.start());
    ASSERT_TRUE(thread.start_push(Buffer("Hello World")));
    EXPECT_EQ(0, thread.end_push());

    // Closing the queue shall not prevent pulling items from it.
    // After that, -EIO shall be returned.
    thread.do_close();

    ASSERT_TRUE(thread.start_push(Buffer("Foo Bar")));
    EXPECT_EQ(-EIO, thread.end_push());

    {
      std::unique_lock<std::mutex> l(lock);
      Buffer buffer;
      EXPECT_EQ(0, queue.pop_locked(&buffer, l));
      EXPECT_STREQ("Hello World", buffer.data());

      EXPECT_EQ(-EIO, queue.pop_locked(&buffer, l));
      EXPECT_STREQ("Hello World", buffer.data());
    }

    thread.stop();
}
} // namespace graphics
} // namespace anbox
