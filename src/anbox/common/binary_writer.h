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

#ifndef ANBOX_COMMON_BINARY_WRITER_H_
#define ANBOX_COMMON_BINARY_WRITER_H_

#include <cstdint>
#include <vector>
#include <string>

namespace anbox {
namespace common {
class BinaryWriter {
 public:
  enum class Order {
    Big,
    Little,
  };

  explicit BinaryWriter(std::vector<std::uint8_t>::iterator begin_,
                        std::vector<std::uint8_t>::iterator end_);

  void set_byte_order(Order order);

  void write_uint16(std::uint16_t value);
  void write_uint32(std::uint32_t value);
  void write_string(const char *s, std::size_t size);
  void write_string_with_size(const std::string &str);
  void write_string_with_size(const char *s, std::size_t size);

  std::size_t bytes_written() const;

 private:
  std::vector<std::uint8_t>::iterator begin_;
  std::vector<std::uint8_t>::iterator current_;
  std::vector<std::uint8_t>::iterator end_;
  Order byte_order_;
};
} // namespace common
} // namespace anbox

#endif
