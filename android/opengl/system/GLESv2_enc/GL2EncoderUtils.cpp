/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <stdio.h>
#include <stdlib.h>
#include "GL2Encoder.h"
#include <assert.h>

namespace glesv2_enc {

size_t pixelDataSize(void *self, GLsizei width, GLsizei height, GLenum format, GLenum type, int pack)
{
    GL2Encoder *ctx = (GL2Encoder *)self;
    assert (ctx->state() != NULL);
    return ctx->state()->pixelDataSize(width, height, format, type, pack);
}

size_t pixelDataSize3D(void *self, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, int pack)
{
    size_t layerSize = pixelDataSize(self, width, height, format, type, pack);
    return layerSize * depth;
}

GLenum uniformType(void * self, GLuint program, GLint location)
{
    GL2Encoder * ctx = (GL2Encoder *) self;
    assert (ctx->shared() != NULL);
    return ctx->shared()->getProgramUniformType(program, location);
}

}  // namespace glesv2_enc
