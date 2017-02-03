/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "anbox/graphics/buffered_io_stream.h"

#include <chrono>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

namespace {
class MockSocketMessenger :
    public anbox::network::SocketMessenger {
 public:
  // anbox::network::SocketMessenger
  MOCK_CONST_METHOD0(creds, anbox::network::Credentials());
  MOCK_CONST_METHOD0(local_port, unsigned short());
  MOCK_METHOD0(set_no_delay, void());
  MOCK_METHOD0(close, void());

  // anbox::network::MessageSender
  MOCK_METHOD2(send, void(char const*, size_t));
  MOCK_METHOD2(send_raw, ssize_t(char const*, size_t));

  // anbox::network::MessageReceiver
  MOCK_METHOD2(async_receive_msg, void(AnboxReadHandler const&, boost::asio::mutable_buffers_1 const&));
  MOCK_METHOD1(receive_msg, boost::system::error_code(boost::asio::mutable_buffers_1 const&));
  MOCK_METHOD0(available_bytes, size_t());
};
}

namespace anbox {
namespace graphics {
TEST(BufferedIOStream, CommitBufferWritesOutToMessenger) {
  auto messenger = std::make_shared<MockSocketMessenger>();
  BufferedIOStream stream(messenger);

  const size_t buffer_size{1000};
  // We will write out the data we get in two junks of half the size
  // the original buffer has.
  EXPECT_CALL(*messenger, send_raw(_, buffer_size))
      .Times(1)
      .WillOnce(Return(buffer_size/2));
  EXPECT_CALL(*messenger, send_raw(_, buffer_size/2))
      .Times(1)
      .WillOnce(Return(buffer_size/2));

  char *ptr = static_cast<char*>(stream.allocBuffer(buffer_size));
  ASSERT_NE(ptr, nullptr);
  ASSERT_EQ(stream.commitBuffer(buffer_size), buffer_size);

  // The BufferedIOStream class works internally with a thread to
  // write out the actual data to the messenger. As it blocks in
  // its d'tor for the writer thread to quit we can safely expect
  // that the messenger will see all data it has to.
}

TEST(BufferedIOStream, WriterContinuesWhenSocketIsBusy) {
  auto messenger = std::make_shared<MockSocketMessenger>();
  BufferedIOStream stream(messenger);

  const size_t buffer_size{1000};
  const size_t first_chunk_size{100};
  // The writer will check the error code of the send function
  // and will retry writing the next chunk when it doesn't get
  // EAGAIN anymore from the sender.
  EXPECT_CALL(*messenger, send_raw(_, buffer_size))
      .Times(1)
      .WillOnce(Return(first_chunk_size));
  EXPECT_CALL(*messenger, send_raw(_, buffer_size - first_chunk_size))
      .Times(2)
      .WillOnce(DoAll(Invoke([](char const*, size_t) { errno = EAGAIN; }), Return(-EAGAIN)))
      .WillOnce(Return(buffer_size - first_chunk_size));

  char *ptr = static_cast<char*>(stream.allocBuffer(buffer_size));
  ASSERT_NE(ptr, nullptr);
  ASSERT_EQ(stream.commitBuffer(buffer_size), buffer_size);
}

TEST(BufferedIOStream, ReadWhenEnoughDataAvailable) {
  auto messenger = std::make_shared<MockSocketMessenger>();
  BufferedIOStream stream(messenger);

  Buffer buffer;
  buffer.push_back(0x12);
  buffer.push_back(0x34);
  stream.post_data(std::move(buffer));

  std::uint8_t read_data[1] = {0x0};
  size_t size = 1;
  EXPECT_NE(nullptr, stream.read(read_data, &size));
  EXPECT_EQ(1, size);
  EXPECT_EQ(0x12, read_data[0]);

  EXPECT_NE(nullptr, stream.read(read_data, &size));
  EXPECT_EQ(1, size);
  EXPECT_EQ(0x34, read_data[0]);
}

TEST(BufferedIOStream, ReadWithNoDataAvailable) {
  auto messenger = std::make_shared<MockSocketMessenger>();
  BufferedIOStream stream(messenger);

  bool stopped = false;
  std::thread producer([&](){
    while (!stopped) {
      if (stream.needs_data()) {
        Buffer buffer;
        buffer.push_back(0x12);
        buffer.push_back(0x34);
        stream.post_data(std::move(buffer));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
  });

  constexpr size_t size{10};
  std::uint8_t read_data[size] = {0x0};
  size_t read = 10;
  EXPECT_NE(nullptr, stream.read(read_data, &read));
  EXPECT_EQ(2, read);
  EXPECT_EQ(0x12, read_data[0]);
  EXPECT_EQ(0x34, read_data[1]);

  stopped = true;
  producer.join();
}
} // namespace graphics
} // namespace anbox
