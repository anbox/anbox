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

TEST(BinaryWriter, WritesUnsignedLong) {
  std::vector<std::uint8_t> buffer;
  buffer.resize(sizeof(std::uint32_t) * 2);
  ac::BinaryWriter writer(buffer.begin(), buffer.end());

  writer.write_unsigned_long(0x10);
  writer.write_unsigned_long(0x3322);

  ASSERT_THAT(buffer, ElementsAre(0x10, 0x00, 0x00, 0x00, 0x22, 0x33, 0x00, 0x00));
}

TEST(BinaryWriter, WriteUnsignedLongFailsWithExhaustedError) {
  std::vector<std::uint8_t> buffer;
  ac::BinaryWriter writer(buffer.begin(), buffer.end());
  EXPECT_THROW(writer.write_unsigned_long(0x11), std::out_of_range);
}

TEST(BinaryWriter, WriteUnsignedLongWithChangedBinaryOrder) {
  std::vector<std::uint8_t> buffer;
  buffer.resize(sizeof(std::uint32_t));
  ac::BinaryWriter writer(buffer.begin(), buffer.end());

  writer.set_byte_order(ac::BinaryWriter::Order::Big);
  writer.write_unsigned_long(0x11223344);

  ASSERT_THAT(buffer, ElementsAre(0x11, 0x22, 0x33, 0x44));

  buffer.clear();
  buffer.resize(sizeof(std::uint32_t));

  writer = ac::BinaryWriter(buffer.begin(), buffer.end());

  writer.set_byte_order(ac::BinaryWriter::Order::Little);
  writer.write_unsigned_long(0x11223344);

  ASSERT_THAT(buffer, ElementsAre(0x44, 0x33, 0x22, 0x11));
}

TEST(BinaryWriter, WriteUnsignedShort) {
  std::vector<std::uint8_t> buffer;
  buffer.resize(sizeof(std::uint16_t) * 2);
  ac::BinaryWriter writer(buffer.begin(), buffer.end());

  writer.write_unsigned_short(0x10);
  writer.write_unsigned_short(0x3322);

  ASSERT_THAT(buffer, ElementsAre(0x10, 0x00, 0x22, 0x33));
}

TEST(BinaryWriter, WriteUnsignedShortFailsWithExhaustedError) {
  std::vector<std::uint8_t> buffer;
  ac::BinaryWriter writer(buffer.begin(), buffer.end());
  EXPECT_THROW(writer.write_unsigned_short(0x11), std::out_of_range);
}

TEST(BinaryWriter, WriteUnsignedShortWithChangedBinaryOrder) {
  std::vector<std::uint8_t> buffer;
  buffer.resize(sizeof(std::uint16_t));
  ac::BinaryWriter writer(buffer.begin(), buffer.end());

  writer.set_byte_order(ac::BinaryWriter::Order::Big);
  writer.write_unsigned_short(0x1122);

  ASSERT_THAT(buffer, ElementsAre(0x11, 0x22));

  buffer.clear();
  buffer.resize(sizeof(std::uint16_t));

  writer = ac::BinaryWriter(buffer.begin(), buffer.end());

  writer.set_byte_order(ac::BinaryWriter::Order::Little);
  writer.write_unsigned_short(0x1122);

  ASSERT_THAT(buffer, ElementsAre(0x22, 0x11));
}
