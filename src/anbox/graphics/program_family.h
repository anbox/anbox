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

#ifndef ANBOX_GRAPHICS_PROGRAM_FAMILY_H_
#define ANBOX_GRAPHICS_PROGRAM_FAMILY_H_

#include <map>
#include <unordered_map>
#include <utility>

#include <GLES2/gl2.h>

namespace anbox {
namespace graphics {
/**
 * ProgramFamily represents a set of GLSL programs that are closely
 * related. Programs which point to the same shader source strings will be
 * made to share the same compiled shader objects.
 *   A secondary intention is that this class may be extended to allow the
 * different programs within the family to share common patterns of uniform
 * usage too.
 */
class ProgramFamily {
 public:
  ProgramFamily() = default;
  ProgramFamily(ProgramFamily const&) = delete;
  ProgramFamily& operator=(ProgramFamily const&) = delete;
  ~ProgramFamily() noexcept;

  GLuint add_program(const GLchar* const static_vshader_src,
                     const GLchar* const static_fshader_src);

 private:
  struct Shader {
    GLuint id = 0;
    void init(GLenum type, const GLchar* src);
  };
  typedef std::unordered_map<const GLchar*, Shader> ShaderMap;
  ShaderMap vshader, fshader;

  typedef std::pair<GLuint, GLuint> ShaderPair;
  struct Program {
    GLuint id = 0;
  };
  std::map<ShaderPair, Program> program;
};
}  // namespace graphics
}  // namespace anbox

#endif  // MIR_RENDERER_GL_PROGRAM_FAMILY_H_
