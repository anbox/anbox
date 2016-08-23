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
#ifndef PROGRAM_DATA_H
#define PROGRAM_DATA_H

class ProgramData:public ObjectData{
public:
    ProgramData();
    virtual ~ProgramData();

    GLuint getAttachedVertexShader();
    GLuint getAttachedFragmentShader();
    GLuint getAttachedShader(GLenum type);

    bool attachShader(GLuint shader,GLenum type);
    bool isAttached(GLuint shader);
    bool detachShader(GLuint shader);

    void setLinkStatus(GLint status);
    GLint getLinkStatus();

    void setInfoLog(GLchar *log);
    GLchar* getInfoLog();

    bool isInUse() const { return IsInUse; }
    void setInUse(bool inUse) { IsInUse = inUse; }

    bool getDeleteStatus() const { return DeleteStatus; }
    void setDeleteStatus(bool status) { DeleteStatus = status; }
private:
    GLuint AttachedVertexShader;
    GLuint AttachedFragmentShader;
    GLint  LinkStatus;
    GLchar* infoLog;
    bool    IsInUse;
    bool    DeleteStatus;
};
#endif
