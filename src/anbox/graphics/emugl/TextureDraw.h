// Copyright (C) 2015 The Android Open Source Project
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

#ifndef TEXTURE_DRAW_H
#define TEXTURE_DRAW_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>

// Helper class used to draw a simple texture to the current framebuffer.
// Usage is pretty simple:
//
//   1) Create a TextureDraw instance, passing the current EGLDisplay to it.
//
//   2) Each time you want to draw a texture, call draw(texture, rotation),
//      where |texture| is the name of a GLES 2.x texture object, and
//      |rotation| is an angle in degrees describing the clockwise rotation
//      in the GL y-upwards coordinate space. This function fills the whole
//      framebuffer with texture content.
//
class TextureDraw {
 public:
  // Create a new instance.
  TextureDraw(EGLDisplay display);

  // Destructor
  ~TextureDraw();

  // Fill the current framebuffer with the content of |texture|, which must
  // be the name of a GLES 2.x texture object.
  bool draw(GLuint texture);

 private:
  GLuint mVertexShader;
  GLuint mFragmentShader;
  GLuint mProgram;
  GLint mPositionSlot;
  GLint mInCoordSlot;
  GLint mTextureSlot;
  GLuint mVertexBuffer;
  GLuint mIndexBuffer;
};

#endif  // TEXTURE_DRAW_H
