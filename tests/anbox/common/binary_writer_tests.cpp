/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
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

#include "anbox/common/binary_writer.h"

#include <gmock/gmock.h>

namespace ac = anbox::common;

using namespace ::testing;

TEST(BinaryWriter, WriteUint32) {
  std::vector<std::uint8_t> buffer;
  buffer.resize(sizeof(std::uint32_t) * 2);
  ac::BinaryWriter writer(buffer.begin(), buffer.end());

  writer.write_uint32(0x10);
  writer.write_uint32(0x3322);

  ASSERT_EQ(writer.bytes_written(), 8);
  ASSERT_THAT(buffer, ElementsAre(0x10, 0x00, 0x00, 0x00, 0x22, 0x33, 0x00, 0x00));
}

TEST(BinaryWriter, WriteUint32FailsWithExhaustedError) {
  std::vector<std::uint8_t> buffer;
  ac::BinaryWriter writer(buffer.begin(), buffer.end());
  EXPECT_THROW(writer.write_uint32(0x11), std::out_of_range);
}

TEST(BinaryWriter, WriteUint32WithChangedBinaryOrder) {
  std::vector<std::uint8_t> buffer;
  buffer.resize(sizeof(std::uint32_t));
  ac::BinaryWriter writer(buffer.begin(), buffer.end());

  writer.set_byte_order(ac::BinaryWriter::Order::Big);
  writer.write_uint32(0x11223344);

  ASSERT_EQ(writer.bytes_written(), 4);
  ASSERT_THAT(buffer, ElementsAre(0x11, 0x22, 0x33, 0x44));

  buffer.clear();
  buffer.resize(sizeof(std::uint32_t));

  writer = ac::BinaryWriter(buffer.begin(), buffer.end());

  writer.set_byte_order(ac::BinaryWriter::Order::Little);
  writer.write_uint32(0x11223344);

  ASSERT_EQ(writer.bytes_written(), 4);
  ASSERT_THAT(buffer, ElementsAre(0x44, 0x33, 0x22, 0x11));
}

TEST(BinaryWriter, WriteUint16) {
  std::vector<std::uint8_t> buffer;
  buffer.resize(sizeof(std::uint16_t) * 2);
  ac::BinaryWriter writer(buffer.begin(), buffer.end());

  writer.write_uint16(0x10);
  writer.write_uint16(0x3322);

  ASSERT_EQ(writer.bytes_written(), 4);
  ASSERT_THAT(buffer, ElementsAre(0x10, 0x00, 0x22, 0x33));
}

TEST(BinaryWriter, WriteUint16FailsWithExhaustedError) {
  std::vector<std::uint8_t> buffer;
  ac::BinaryWriter writer(buffer.begin(), buffer.end());
  EXPECT_THROW(writer.write_uint16(0x11), std::out_of_range);
}

TEST(BinaryWriter, WriteUint16WithChangedBinaryOrder) {
  std::vector<std::uint8_t> buffer;
  buffer.resize(sizeof(std::uint16_t));
  ac::BinaryWriter writer(buffer.begin(), buffer.end());

  writer.set_byte_order(ac::BinaryWriter::Order::Big);
  writer.write_uint16(0x1122);

  ASSERT_EQ(writer.bytes_written(), 2);
  ASSERT_THAT(buffer, ElementsAre(0x11, 0x22));

  buffer.clear();
  buffer.resize(sizeof(std::uint16_t));

  writer = ac::BinaryWriter(buffer.begin(), buffer.end());

  writer.set_byte_order(ac::BinaryWriter::Order::Little);
  writer.write_uint16(0x1122);

  ASSERT_EQ(writer.bytes_written(), 2);
  ASSERT_THAT(buffer, ElementsAre(0x22, 0x11));
}

TEST(BinaryWriter, WriteString) {
  std::vector<std::uint8_t> buffer;
  buffer.resize(sizeof(std::uint8_t) * 4);
  ac::BinaryWriter writer(buffer.begin(), buffer.end());

  writer.write_string("test", 4);

  ASSERT_EQ(writer.bytes_written(), 4);
  ASSERT_THAT(buffer, ElementsAre(0x74, 0x65, 0x73, 0x74));
}

TEST(BinaryWriter, WriteStringWithSize) {
  std::vector<std::uint8_t> buffer;
  buffer.resize(sizeof(std::uint8_t) * 6);
  ac::BinaryWriter writer(buffer.begin(), buffer.end());

  writer.set_byte_order(ac::BinaryWriter::Order::Big);

  writer.write_string_with_size("test");

  ASSERT_EQ(writer.bytes_written(), 6);
  ASSERT_THAT(buffer, ElementsAre(0x00, 0x04, 0x74, 0x65, 0x73, 0x74));
}
