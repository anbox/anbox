/*
 * Copyright © 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Daniel van Vugt <daniel.van.vugt@canonical.com>
 */

#include "anbox/graphics/program_family.h"

#include "anbox/graphics/emugl/DispatchTables.h"

#include <string>
#include <stdexcept>

namespace anbox {
namespace graphics {
void ProgramFamily::Shader::init(GLenum type, const GLchar* src) {
  if (!id) {
    id = s_gles2.glCreateShader(type);
    if (id) {
      s_gles2.glShaderSource(id, 1, &src, NULL);
      s_gles2.glCompileShader(id);
      GLint ok;
      s_gles2.glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
      if (!ok) {
        GLchar log[1024];
        s_gles2.glGetShaderInfoLog(id, sizeof log - 1, NULL, log);
        log[sizeof log - 1] = '\0';
        s_gles2.glDeleteShader(id);
        id = 0;
        throw std::runtime_error(std::string("Compile failed: ") + log +
                                 " for:\n" + src);
      }
    }
  }
}

ProgramFamily::~ProgramFamily() noexcept {
  // shader and program lifetimes are managed manually, so that we don't
  // need any reference counting or to worry about how many copy constructions
  // might have been followed by destructor calls during container resizes.

  for (auto& p : program) {
    if (p.second.id) s_gles2.glDeleteProgram(p.second.id);
  }

  for (auto& v : vshader) {
    if (v.second.id) s_gles2.glDeleteShader(v.second.id);
  }

  for (auto& f : fshader) {
    if (f.second.id) s_gles2.glDeleteShader(f.second.id);
  }
}

GLuint ProgramFamily::add_program(const GLchar* const vshader_src,
                                  const GLchar* const fshader_src) {
  auto& v = vshader[vshader_src];
  if (!v.id) v.init(GL_VERTEX_SHADER, vshader_src);

  auto& f = fshader[fshader_src];
  if (!f.id) f.init(GL_FRAGMENT_SHADER, fshader_src);

  auto& p = program[{v.id, f.id}];
  if (!p.id) {
    p.id = s_gles2.glCreateProgram();
    s_gles2.glAttachShader(p.id, v.id);
    s_gles2.glAttachShader(p.id, f.id);
    s_gles2.glLinkProgram(p.id);
    GLint ok;
    s_gles2.glGetProgramiv(p.id, GL_LINK_STATUS, &ok);
    if (!ok) {
      GLchar log[1024];
      s_gles2.glGetProgramInfoLog(p.id, sizeof log - 1, NULL, log);
      log[sizeof log - 1] = '\0';
      s_gles2.glDeleteShader(p.id);
      p.id = 0;
      throw std::runtime_error(std::string("Link failed: ") + log);
    }
  }

  return p.id;
}
}  // namespace graphics
}  // namespace anbox
