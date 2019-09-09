/*
 * Copyright (C) 2016 Thomas Voss <thomas.voss.bochum@gmail.com>
 *                    Simon Fels <morphis@gravedo.de>
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

#include <cstring>
#include <stdexcept>

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>

namespace {
bool is_little_endian() {
  static const std::uint32_t v = 1;
  return (*reinterpret_cast<const std::uint8_t*>(&v) ==  1);
}
}

namespace anbox {
namespace common {

BinaryWriter::BinaryWriter(std::vector<std::uint8_t>::iterator begin,
                           std::vector<std::uint8_t>::iterator end) :
  begin_{begin}, current_{begin}, end_{end},
  byte_order_{is_little_endian() ? Order::Little : Order::Big} {}

void BinaryWriter::set_byte_order(Order order) {
  byte_order_ = order;
}

void BinaryWriter::write_uint16(std::uint16_t value) {
  if (current_ + sizeof(value) > end_)
    throw std::out_of_range{"Write buffer exhausted"};

  std::uint16_t v = value;
  switch (byte_order_) {
  case Order::Big:
    v = boost::endian::native_to_big(value);
    break;
  case Order::Little:
    v = boost::endian::native_to_little(value);
    break;
  default:
    break;
  }

  memcpy(&(*current_), &v, sizeof(std::uint16_t));
  current_ += sizeof(v);
}

void BinaryWriter::write_uint32(std::uint32_t value) {
  if (current_ + sizeof(value) > end_)
    throw std::out_of_range{"Write buffer exhausted"};

  std::uint32_t v = value;
  switch (byte_order_) {
  case Order::Big:
    v = boost::endian::native_to_big(value);
    break;
  case Order::Little:
    v = boost::endian::native_to_little(value);
    break;
  default:
    break;
  }
  memcpy(&(*current_), &v, sizeof(std::uint32_t));
  current_ += sizeof(v);
}

void BinaryWriter::write_string(const char *s, std::size_t size) {
  if (current_ + size > end_)
    throw std::out_of_range{"Write buffer exhausted"};

  memcpy(&(*current_), s, size);
  current_ += size;
}

void BinaryWriter::write_string_with_size(const std::string &str) {
  write_string_with_size(str.c_str(), str.length());
}

void BinaryWriter::write_string_with_size(const char *s, std::size_t size) {
  write_uint16(size);
  write_string(s, size);
}

std::size_t BinaryWriter::bytes_written() const {
  return current_ - begin_;
}
} // namespace common
} // namespace anbox
