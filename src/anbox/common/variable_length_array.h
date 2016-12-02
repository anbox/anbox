/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#ifndef ANBOX_COMMON_VARIABLE_LENGTH_ARRAY_H_
#define ANBOX_COMMON_VARIABLE_LENGTH_ARRAY_H_

#include <sys/types.h>
#include <memory>

namespace anbox {
template <size_t BuiltInBufferSize>
class VariableLengthArray {
 public:
  explicit VariableLengthArray(size_t size) : size_{size} {
    /* Don't call resize if the initial values of member variables are valid */
    if (size > BuiltInBufferSize) resize(size);
  }

  void resize(size_t size) {
    if (size > BuiltInBufferSize)
      effective_buffer = BufferUPtr{new unsigned char[size], heap_deleter};
    else
      effective_buffer = BufferUPtr{builtin_buffer, null_deleter};

    size_ = size;
  }

  unsigned char* data() const { return effective_buffer.get(); }
  size_t size() const { return size_; }

 private:
  typedef std::unique_ptr<unsigned char, void (*)(unsigned char*)> BufferUPtr;

  static void null_deleter(unsigned char*) {}
  static void heap_deleter(unsigned char* b) { delete[] b; }

  unsigned char builtin_buffer[BuiltInBufferSize];
  BufferUPtr effective_buffer{builtin_buffer, null_deleter};
  size_t size_;
};
}  // namespace anbox

#endif
