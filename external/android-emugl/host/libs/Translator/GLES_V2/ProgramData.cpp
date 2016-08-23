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
#include <GLES2/gl2.h>
#include <GLcommon/objectNameManager.h>
#include "ProgramData.h"

ProgramData::ProgramData() :  ObjectData(PROGRAM_DATA),
                              AttachedVertexShader(0),
                              AttachedFragmentShader(0),
                              LinkStatus(GL_FALSE),
                              IsInUse(false),
                              DeleteStatus(false) {
    infoLog = new GLchar[1];
    infoLog[0] = '\0';
}

ProgramData::~ProgramData () {
    delete[] infoLog;
};

void ProgramData::setInfoLog(GLchar* log) {
    delete[] infoLog;
    infoLog = log;
}

GLchar* ProgramData::getInfoLog() {
    return infoLog;
}

GLuint ProgramData::getAttachedVertexShader() {
    return AttachedVertexShader;
}

GLuint ProgramData::getAttachedFragmentShader() {
    return AttachedFragmentShader;
}

GLuint ProgramData::getAttachedShader(GLenum type) {
    GLuint shader = 0;
    switch (type) {
    case GL_VERTEX_SHADER:
        shader = AttachedVertexShader;
        break;
    case GL_FRAGMENT_SHADER:
        shader = AttachedFragmentShader;
        break;
    }
    return shader;
}

bool ProgramData::attachShader(GLuint shader,GLenum type) {
    if (type==GL_VERTEX_SHADER && AttachedVertexShader==0) {
        AttachedVertexShader=shader;
        return true;
    }
    else if (type==GL_FRAGMENT_SHADER && AttachedFragmentShader==0) {
        AttachedFragmentShader=shader;
        return true;
    }
    return false;
}

bool ProgramData::isAttached(GLuint shader) {
    return (AttachedFragmentShader==shader || AttachedVertexShader==shader);
}

bool ProgramData::detachShader(GLuint shader) {
    if (AttachedVertexShader==shader) {
        AttachedVertexShader = 0;
        return true;
    }
    else if (AttachedFragmentShader==shader) {
        AttachedFragmentShader = 0;
        return true;
    }
    return false;
}

void ProgramData::setLinkStatus(GLint status) {
    LinkStatus = status;
}

GLint ProgramData::getLinkStatus() {
    return LinkStatus;
}
