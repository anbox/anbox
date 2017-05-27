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

#include "anbox/graphics/emugl/TextureDraw.h"
#include "anbox/graphics/emugl/DispatchTables.h"
#include "anbox/logger.h"

#include <math.h>
#include <string.h>
#include <vector>

#include <stdio.h>

// M_PI isn't defined in C++ (when strict ISO compliance is enabled)
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

namespace {

// Helper function to create a new shader.
// |shaderType| is the shader type (e.g. GL_VERTEX_SHADER).
// |shaderText| is a 0-terminated C string for the shader source to use.
// On success, return the handle of the new compiled shader, or 0 on failure.
GLuint createShader(GLint shaderType, const char* shaderText) {
  // Create new shader handle and attach source.
  GLuint shader = s_gles2.glCreateShader(shaderType);
  if (!shader) {
    return 0;
  }
  const GLchar* text = static_cast<const GLchar*>(shaderText);
  const GLint textLen = ::strlen(shaderText);
  s_gles2.glShaderSource(shader, 1, &text, &textLen);

  // Compiler the shader.
  GLint success;
  s_gles2.glCompileShader(shader);
  s_gles2.glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    s_gles2.glDeleteShader(shader);
    return 0;
  }

  return shader;
}

// No scaling / projection since we want to fill the whole viewport with
// the texture, hence a trivial vertex shader.
const char kVertexShaderSource[] =
    "attribute vec4 position;\n"
    "attribute vec2 inCoord;\n"
    "varying lowp vec2 outCoord;\n"
    "void main(void) {\n"
    "  gl_Position.x = position.x;\n"
    "  gl_Position.y = position.y;\n"
    "  gl_Position.zw = position.zw;\n"
    "  outCoord = inCoord;\n"
    "}\n";

// Similarly, just interpolate texture coordinates.
const char kFragmentShaderSource[] =
    "varying lowp vec2 outCoord;\n"
    "uniform sampler2D texture;\n"

    "void main(void) {\n"
    "  gl_FragColor = texture2D(texture, outCoord);\n"
    "}\n";

// Hard-coded arrays of vertex information.
struct Vertex {
  float pos[3];
  float coord[2];
};

const Vertex kVertices[] = {
    {{+1, -1, +0}, {+1, +1}},
    {{+1, +1, +0}, {+1, +0}},
    {{-1, +1, +0}, {+0, +0}},
    {{-1, -1, +0}, {+0, +1}},
};

const GLubyte kIndices[] = {0, 1, 2, 2, 3, 0};
const GLint kIndicesLen = sizeof(kIndices) / sizeof(kIndices[0]);

}  // namespace

TextureDraw::TextureDraw(EGLDisplay)
    : mVertexShader(0),
      mFragmentShader(0),
      mProgram(0),
      mPositionSlot(-1),
      mInCoordSlot(-1),
      mTextureSlot(-1) {
  // Create shaders and program.
  mVertexShader = createShader(GL_VERTEX_SHADER, kVertexShaderSource);
  mFragmentShader = createShader(GL_FRAGMENT_SHADER, kFragmentShaderSource);

  mProgram = s_gles2.glCreateProgram();
  s_gles2.glAttachShader(mProgram, mVertexShader);
  s_gles2.glAttachShader(mProgram, mFragmentShader);

  GLint success;
  s_gles2.glLinkProgram(mProgram);
  s_gles2.glGetProgramiv(mProgram, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    GLchar messages[256];
    s_gles2.glGetProgramInfoLog(mProgram, sizeof(messages), 0, &messages[0]);
    ERROR("Could not create/link program: %s", messages);
    s_gles2.glDeleteProgram(mProgram);
    mProgram = 0;
    return;
  }

  s_gles2.glUseProgram(mProgram);

  // Retrieve attribute/uniform locations.
  mPositionSlot = s_gles2.glGetAttribLocation(mProgram, "position");
  s_gles2.glEnableVertexAttribArray(mPositionSlot);

  mInCoordSlot = s_gles2.glGetAttribLocation(mProgram, "inCoord");
  s_gles2.glEnableVertexAttribArray(mInCoordSlot);

  mTextureSlot = s_gles2.glGetUniformLocation(mProgram, "texture");

  // Create vertex and index buffers.
  s_gles2.glGenBuffers(1, &mVertexBuffer);
  s_gles2.glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  s_gles2.glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices,
                       GL_STATIC_DRAW);

  s_gles2.glGenBuffers(1, &mIndexBuffer);
  s_gles2.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
  s_gles2.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices,
                       GL_STATIC_DRAW);
}

bool TextureDraw::draw(GLuint texture) {
  if (!mProgram) {
    ERROR(" No program");
    return false;
  }

  // TODO(digit): Save previous program state.

  GLenum err;

  s_gles2.glUseProgram(mProgram);
  err = s_gles2.glGetError();
  if (err != GL_NO_ERROR) {
    ERROR("Could not use program error 0x%x", err);
  }

  // Setup the |position| attribute values.
  s_gles2.glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  err = s_gles2.glGetError();
  if (err != GL_NO_ERROR) {
    ERROR("Could not bind GL_ARRAY_BUFFER error=0x%x", err);
  }

  s_gles2.glEnableVertexAttribArray(mPositionSlot);
  s_gles2.glVertexAttribPointer(mPositionSlot, 3, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex), 0);
  err = s_gles2.glGetError();
  if (err != GL_NO_ERROR) {
    ERROR("Could glVertexAttribPointer with mPositionSlot error 0x%x", err);
  }

  // Setup the |inCoord| attribute values.
  s_gles2.glEnableVertexAttribArray(mInCoordSlot);
  s_gles2.glVertexAttribPointer(
      mInCoordSlot, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<GLvoid*>(static_cast<uintptr_t>(sizeof(float) * 3)));

  // setup the |texture| uniform value.
  s_gles2.glActiveTexture(GL_TEXTURE0);
  s_gles2.glBindTexture(GL_TEXTURE_2D, texture);
  s_gles2.glUniform1i(mTextureSlot, 0);

  // Validate program, just to be sure.
  s_gles2.glValidateProgram(mProgram);
  GLint validState = 0;
  s_gles2.glGetProgramiv(mProgram, GL_VALIDATE_STATUS, &validState);
  if (validState == GL_FALSE) {
    GLchar messages[256];
    s_gles2.glGetProgramInfoLog(mProgram, sizeof(messages), 0, &messages[0]);
    ERROR("Could not run program: %s", messages);
    return false;
  }

  // Do the rendering.
  s_gles2.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
  err = s_gles2.glGetError();
  if (err != GL_NO_ERROR) {
    ERROR("Could not glBindBuffer(GL_ELEMENT_ARRAY_BUFFER) error 0x%x", err);
  }

  s_gles2.glDrawElements(GL_TRIANGLES, kIndicesLen, GL_UNSIGNED_BYTE, 0);
  err = s_gles2.glGetError();
  if (err != GL_NO_ERROR) {
    ERROR("Could not glDrawElements() error 0x%x", err);
  }

  // TODO(digit): Restore previous program state.

  return true;
}

TextureDraw::~TextureDraw() {
  s_gles2.glDeleteBuffers(1, &mIndexBuffer);
  s_gles2.glDeleteBuffers(1, &mVertexBuffer);

  if (mFragmentShader) {
    s_gles2.glDeleteShader(mFragmentShader);
  }
  if (mVertexShader) {
    s_gles2.glDeleteShader(mVertexShader);
  }
}
