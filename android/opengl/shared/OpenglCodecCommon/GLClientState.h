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
#ifndef _GL_CLIENT_STATE_H_
#define _GL_CLIENT_STATE_H_

#define GL_API
#ifndef ANDROID
#define GL_APIENTRY
#define GL_APIENTRYP
#endif

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include "ErrorLog.h"
#include "codec_defs.h"

class GLClientState {
public:
    typedef enum {
        VERTEX_LOCATION = 0,
        NORMAL_LOCATION = 1,
        COLOR_LOCATION = 2,
        POINTSIZE_LOCATION = 3,
        TEXCOORD0_LOCATION = 4,
        TEXCOORD1_LOCATION = 5,
        TEXCOORD2_LOCATION = 6,
        TEXCOORD3_LOCATION = 7,
        TEXCOORD4_LOCATION = 8,
        TEXCOORD5_LOCATION = 9,
        TEXCOORD6_LOCATION = 10,
        TEXCOORD7_LOCATION = 11,
        MATRIXINDEX_LOCATION = 12,
        WEIGHT_LOCATION = 13,
        LAST_LOCATION = 14
    } StateLocation;

    typedef struct {
        GLint enabled;
        GLint size;
        GLenum type;
        GLsizei stride;
        void *data;
        GLuint bufferObject;
        GLenum glConst;
        unsigned int elementSize;
        bool enableDirty;  // true if any enable state has changed since last draw
        bool normalized;
    } VertexAttribState;

    typedef struct {
        int unpack_alignment;
        int pack_alignment;
    } PixelStoreState;

    enum {
        MAX_TEXTURE_UNITS = 32,
    };

public:
    GLClientState(int nLocations = CODEC_MAX_VERTEX_ATTRIBUTES);
    ~GLClientState();
    int nLocations() { return m_nLocations; }
    const PixelStoreState *pixelStoreState() { return &m_pixelStore; }
    int setPixelStore(GLenum param, GLint value);
    GLuint currentArrayVbo() { return m_currentArrayVbo; }
    GLuint currentIndexVbo() { return m_currentIndexVbo; }
    void enable(int location, int state);
    void setState(int  location, int size, GLenum type, GLboolean normalized, GLsizei stride, const void *data);
    void setBufferObject(int location, GLuint id);
    const VertexAttribState  *getState(int location);
    const VertexAttribState  *getStateAndEnableDirty(int location, bool *enableChanged);
    int getLocation(GLenum loc);
    void setActiveTexture(int texUnit) {m_activeTexture = texUnit; };
    int getActiveTexture() const { return m_activeTexture; }
    void setMaxVertexAttribs(int val) {
        m_maxVertexAttribs = val;
        m_maxVertexAttribsDirty = false;
    }

    void unBindBuffer(GLuint id)
    {
        if (m_currentArrayVbo == id) m_currentArrayVbo = 0;
        else if (m_currentIndexVbo == id) m_currentIndexVbo = 0;
    }

    int bindBuffer(GLenum target, GLuint id)
    {
        int err = 0;
        switch(target) {
        case GL_ARRAY_BUFFER:
            m_currentArrayVbo = id;
            break;
        case GL_ELEMENT_ARRAY_BUFFER:
            m_currentIndexVbo = id;
            break;
        default:
            err = -1;
        }
        return err;
    }

    int getBuffer(GLenum target)
    {
      int ret=0;
      switch (target) {
      case GL_ARRAY_BUFFER:
          ret = m_currentArrayVbo;
          break;
      case GL_ELEMENT_ARRAY_BUFFER:
          ret = m_currentIndexVbo;
          break;
      default:
          ret = -1;
      }
      return ret;
    }
    size_t pixelDataSize(GLsizei width, GLsizei height, GLenum format, GLenum type, int pack) const;

    void setCurrentProgram(GLint program) { m_currentProgram = program; }
    GLint currentProgram() const { return m_currentProgram; }

    /* OES_EGL_image_external
     *
     * These functions manipulate GL state which interacts with the
     * OES_EGL_image_external extension, to support client-side emulation on
     * top of host implementations that don't have it.
     *
     * Most of these calls should only be used with TEXTURE_2D or
     * TEXTURE_EXTERNAL_OES texture targets; TEXTURE_CUBE_MAP or other extension
     * targets should bypass this. An exception is bindTexture(), which should
     * see all glBindTexture() calls for any target.
     */

    // glActiveTexture(GL_TEXTURE0 + i)
    // Sets the active texture unit. Up to MAX_TEXTURE_UNITS are supported.
    GLenum setActiveTextureUnit(GLenum texture);
    GLenum getActiveTextureUnit() const;

    // glEnable(GL_TEXTURE_(2D|EXTERNAL_OES))
    void enableTextureTarget(GLenum target);

    // glDisable(GL_TEXTURE_(2D|EXTERNAL_OES))
    void disableTextureTarget(GLenum target);

    // Implements the target priority logic:
    // * Return GL_TEXTURE_EXTERNAL_OES if enabled, else
    // * Return GL_TEXTURE_2D if enabled, else
    // * Return the allDisabled value.
    // For some cases passing GL_TEXTURE_2D for allDisabled makes callee code
    // simpler; for other cases passing a recognizable enum like GL_ZERO or
    // GL_INVALID_ENUM is appropriate.
    GLenum getPriorityEnabledTarget(GLenum allDisabled) const;

    // glBindTexture(GL_TEXTURE_*, ...)
    // Set the target binding of the active texture unit to texture. Returns
    // GL_NO_ERROR on success or GL_INVALID_OPERATION if the texture has
    // previously been bound to a different target. If firstUse is not NULL,
    // it is set to indicate whether this is the first use of the texture.
    // For accurate error detection, bindTexture should be called for *all*
    // targets, not just 2D and EXTERNAL_OES.
    GLenum bindTexture(GLenum target, GLuint texture, GLboolean* firstUse);

    // Return the texture currently bound to GL_TEXTURE_(2D|EXTERNAL_OES).
    GLuint getBoundTexture(GLenum target) const;

    // glDeleteTextures(...)
    // Remove references to the to-be-deleted textures.
    void deleteTextures(GLsizei n, const GLuint* textures);

private:
    PixelStoreState m_pixelStore;
    VertexAttribState *m_states;
    int m_maxVertexAttribs;
    bool m_maxVertexAttribsDirty;
    int m_nLocations;
    GLuint m_currentArrayVbo;
    GLuint m_currentIndexVbo;
    int m_activeTexture;
    GLint m_currentProgram;

    bool validLocation(int location) { return (location >= 0 && location < m_nLocations); }

    enum TextureTarget {
        TEXTURE_2D = 0,
        TEXTURE_EXTERNAL = 1,
        TEXTURE_TARGET_COUNT
    };
    struct TextureUnit {
        unsigned int enables;
        GLuint texture[TEXTURE_TARGET_COUNT];
    };
    struct TextureRec {
        GLuint id;
        GLenum target;
    };
    struct TextureState {
        TextureUnit unit[MAX_TEXTURE_UNITS];
        TextureUnit* activeUnit;
        TextureRec* textures;
        GLuint numTextures;
        GLuint allocTextures;
    };
    TextureState m_tex;

    static int compareTexId(const void* pid, const void* prec);
    TextureRec* addTextureRec(GLuint id, GLenum target);

public:
    void getClientStatePointer(GLenum pname, GLvoid** params);

    template <class T>
    int getVertexAttribParameter(GLuint index, GLenum param, T *ptr)
    {
        bool handled = true;
        const VertexAttribState *vertexAttrib = getState(index);
        if (vertexAttrib == NULL) {
            ERR("getVeterxAttriParameter for non existant index %d\n", index);
            // set gl error;
            return handled;
        }

        switch(param) {
        case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
            *ptr = (T)(vertexAttrib->bufferObject);
            break;
        case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
            *ptr = (T)(vertexAttrib->enabled);
            break;
        case GL_VERTEX_ATTRIB_ARRAY_SIZE:
            *ptr = (T)(vertexAttrib->size);
            break;
        case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
            *ptr = (T)(vertexAttrib->stride);
            break;
        case GL_VERTEX_ATTRIB_ARRAY_TYPE:
            *ptr = (T)(vertexAttrib->type);
            break;
        case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
            *ptr = (T)(vertexAttrib->normalized);
            break;
        case GL_CURRENT_VERTEX_ATTRIB:
            handled = false;
            break;
        default:
            handled = false;
            ERR("unknown vertex-attrib parameter param %d\n", param);
        }
        return handled;
    }

    template <class T>
    bool getClientStateParameter(GLenum param, T* ptr)
    {
        bool isClientStateParam = false;
        switch (param) {
        case GL_CLIENT_ACTIVE_TEXTURE: {
            GLint tex = getActiveTexture() + GL_TEXTURE0;
            *ptr = tex;
            isClientStateParam = true;
            break;
            }
        case GL_VERTEX_ARRAY_SIZE: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::VERTEX_LOCATION);
            *ptr = state->size;
            isClientStateParam = true;
            break;
            }
        case GL_VERTEX_ARRAY_TYPE: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::VERTEX_LOCATION);
            *ptr = state->type;
            isClientStateParam = true;
            break;
            }
        case GL_VERTEX_ARRAY_STRIDE: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::VERTEX_LOCATION);
            *ptr = state->stride;
            isClientStateParam = true;
            break;
            }
        case GL_COLOR_ARRAY_SIZE: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::COLOR_LOCATION);
            *ptr = state->size;
            isClientStateParam = true;
            break;
            }
        case GL_COLOR_ARRAY_TYPE: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::COLOR_LOCATION);
            *ptr = state->type;
            isClientStateParam = true;
            break;
            }
        case GL_COLOR_ARRAY_STRIDE: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::COLOR_LOCATION);
            *ptr = state->stride;
            isClientStateParam = true;
            break;
            }
        case GL_NORMAL_ARRAY_TYPE: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::NORMAL_LOCATION);
            *ptr = state->type;
            isClientStateParam = true;
            break;
            }
        case GL_NORMAL_ARRAY_STRIDE: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::NORMAL_LOCATION);
            *ptr = state->stride;
            isClientStateParam = true;
            break;
            }
        case GL_TEXTURE_COORD_ARRAY_SIZE: {
            const GLClientState::VertexAttribState *state = getState(getActiveTexture() + GLClientState::TEXCOORD0_LOCATION);
            *ptr = state->size;
            isClientStateParam = true;
            break;
            }
        case GL_TEXTURE_COORD_ARRAY_TYPE: {
            const GLClientState::VertexAttribState *state = getState(getActiveTexture() + GLClientState::TEXCOORD0_LOCATION);
            *ptr = state->type;
            isClientStateParam = true;
            break;
            }
        case GL_TEXTURE_COORD_ARRAY_STRIDE: {
            const GLClientState::VertexAttribState *state = getState(getActiveTexture() + GLClientState::TEXCOORD0_LOCATION);
            *ptr = state->stride;
            isClientStateParam = true;
            break;
            }
        case GL_POINT_SIZE_ARRAY_TYPE_OES: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::POINTSIZE_LOCATION);
            *ptr = state->type;
            isClientStateParam = true;
            break;
            }
        case GL_POINT_SIZE_ARRAY_STRIDE_OES: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::POINTSIZE_LOCATION);
            *ptr = state->stride;
            isClientStateParam = true;
            break;
            }
        case GL_MATRIX_INDEX_ARRAY_SIZE_OES: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::MATRIXINDEX_LOCATION);
            *ptr = state->size;
            isClientStateParam = true;
            break;
            }
        case GL_MATRIX_INDEX_ARRAY_TYPE_OES: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::MATRIXINDEX_LOCATION);
            *ptr = state->type;
            isClientStateParam = true;
            break;
            }
        case GL_MATRIX_INDEX_ARRAY_STRIDE_OES: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::MATRIXINDEX_LOCATION);
            *ptr = state->stride;
            isClientStateParam = true;
            break;
            }
        case GL_WEIGHT_ARRAY_SIZE_OES: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::WEIGHT_LOCATION);
            *ptr = state->size;
            isClientStateParam = true;
            break;
            }
        case GL_WEIGHT_ARRAY_TYPE_OES: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::WEIGHT_LOCATION);
            *ptr = state->type;
            isClientStateParam = true;
            break;
            }
        case GL_WEIGHT_ARRAY_STRIDE_OES: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::WEIGHT_LOCATION);
            *ptr = state->stride;
            isClientStateParam = true;
            break;
            }
        case GL_VERTEX_ARRAY_BUFFER_BINDING: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::VERTEX_LOCATION);
            *ptr = state->bufferObject;
            isClientStateParam = true;
            break;
            }
        case GL_NORMAL_ARRAY_BUFFER_BINDING: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::NORMAL_LOCATION);
            *ptr = state->bufferObject;
            isClientStateParam = true;
            break;
            }
        case GL_COLOR_ARRAY_BUFFER_BINDING: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::COLOR_LOCATION);
            *ptr = state->bufferObject;
            isClientStateParam = true;
            break;
            }
        case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING: {
            const GLClientState::VertexAttribState *state = getState(getActiveTexture()+GLClientState::TEXCOORD0_LOCATION);
            *ptr = state->bufferObject;
            isClientStateParam = true;
            break;
            }
        case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::POINTSIZE_LOCATION);
            *ptr = state->bufferObject;
            isClientStateParam = true;
            break;
            }
        case GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::MATRIXINDEX_LOCATION);
            *ptr = state->bufferObject;
            isClientStateParam = true;
            break;
            }
        case GL_WEIGHT_ARRAY_BUFFER_BINDING_OES: {
            const GLClientState::VertexAttribState *state = getState(GLClientState::WEIGHT_LOCATION);
            *ptr = state->bufferObject;
            isClientStateParam = true;
            break;
            }
        case GL_ARRAY_BUFFER_BINDING: {
            int buffer = getBuffer(GL_ARRAY_BUFFER);
            *ptr = buffer;
            isClientStateParam = true;
            break;
            }
        case GL_ELEMENT_ARRAY_BUFFER_BINDING: {
            int buffer = getBuffer(GL_ELEMENT_ARRAY_BUFFER);
            *ptr = buffer;
            isClientStateParam = true;
            break;
            }
        case GL_MAX_VERTEX_ATTRIBS: {
            if (m_maxVertexAttribsDirty) {
                isClientStateParam = false;
            } else {
                *ptr = m_maxVertexAttribs;
                isClientStateParam = true;
            }
            break;
            }
        }
        return isClientStateParam;
    }

};
#endif
