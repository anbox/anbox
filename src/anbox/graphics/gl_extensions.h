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

#ifndef ANBOX_GRAPHICS_GL_EXTENSIONS_H_
#define ANBOX_GRAPHICS_GL_EXTENSIONS_H_

#include <stdexcept>
#include <string.h>

namespace anbox {
namespace graphics {
class GLExtensions {
 public:
  GLExtensions(char const* extensions) : extensions{extensions} {
    if (!extensions)
      throw std::runtime_error("Couldn't get list of GL extensions");
  }

  bool support(char const* ext) const {
    if (!ext)
      throw std::invalid_argument("Invalid extension name");

    char const* ext_ptr = extensions;
    size_t const len = strlen(ext);
    while ((ext_ptr = strstr(ext_ptr, ext)) != nullptr) {
      if (ext_ptr[len] == ' ' || ext_ptr[len] == '\0')
        break;
      ext_ptr += len;
    }

    return ext_ptr != nullptr;
  }

  char const* raw() { return extensions; }

 private:
  char const* extensions;
};
}  // namespace graphics
}  // namespace anbox

#endif
