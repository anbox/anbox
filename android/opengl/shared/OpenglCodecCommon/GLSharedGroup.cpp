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

#include "GLSharedGroup.h"

/**** KeyedVector utilities ****/

template <typename T>
static void clearObjectMap(android::DefaultKeyedVector<GLuint, T>& v) {
    for (size_t i = 0; i < v.size(); i++)
        delete v.valueAt(i);
    v.clear();
}

/**** BufferData ****/

BufferData::BufferData() : m_size(0) {};
BufferData::BufferData(GLsizeiptr size, void * data) : m_size(size)
{
    void * buffer = NULL;
    if (size>0) buffer = m_fixedBuffer.alloc(size);
    if (data) memcpy(buffer, data, size);
}

/**** ProgramData ****/
ProgramData::ProgramData() : m_numIndexes(0),
                             m_initialized(false),
                             m_locShiftWAR(false)
{
    m_Indexes = NULL;
}

void ProgramData::initProgramData(GLuint numIndexes)
{
    m_initialized = true;
    m_numIndexes = numIndexes;
    delete[] m_Indexes;
    m_Indexes = new IndexInfo[numIndexes];
    m_locShiftWAR = false;
}

bool ProgramData::isInitialized()
{
    return m_initialized;
}

ProgramData::~ProgramData()
{
    delete[] m_Indexes;
    m_Indexes = NULL;
}

void ProgramData::setIndexInfo(GLuint index, GLint base, GLint size, GLenum type)
{
    if (index>=m_numIndexes)
        return;
    m_Indexes[index].base = base;
    m_Indexes[index].size = size;
    m_Indexes[index].type = type;
    if (index > 0) {
        m_Indexes[index].appBase = m_Indexes[index-1].appBase +
                                   m_Indexes[index-1].size;
    }
    else {
        m_Indexes[index].appBase = 0;
    }
    m_Indexes[index].hostLocsPerElement = 1;
    m_Indexes[index].flags = 0;
    m_Indexes[index].samplerValue = 0;
}

void ProgramData::setIndexFlags(GLuint index, GLuint flags)
{
    if (index >= m_numIndexes)
        return;
    m_Indexes[index].flags |= flags;
}

GLuint ProgramData::getIndexForLocation(GLint location)
{
    GLuint index = m_numIndexes;
    GLint minDist = -1;
    for (GLuint i=0;i<m_numIndexes;++i)
    {
        GLint dist = location - m_Indexes[i].base;
        if (dist >= 0 &&
            (minDist < 0 || dist < minDist)) {
            index = i;
            minDist = dist;
        }
    }
    return index;
}

GLenum ProgramData::getTypeForLocation(GLint location)
{
    GLuint index = getIndexForLocation(location);
    if (index<m_numIndexes) {
        return m_Indexes[index].type;
    }
    return 0;
}

void ProgramData::setupLocationShiftWAR()
{
    m_locShiftWAR = false;
    for (GLuint i=0; i<m_numIndexes; i++) {
        if (0 != (m_Indexes[i].base & 0xffff)) {
            return;
        }
    }
    // if we have one uniform at location 0, we do not need the WAR.
    if (m_numIndexes > 1) {
        m_locShiftWAR = true;
    }
}

GLint ProgramData::locationWARHostToApp(GLint hostLoc, GLint arrIndex)
{
    if (!m_locShiftWAR) return hostLoc;

    GLuint index = getIndexForLocation(hostLoc);
    if (index<m_numIndexes) {
        if (arrIndex > 0) {
            m_Indexes[index].hostLocsPerElement =
                              (hostLoc - m_Indexes[index].base) / arrIndex;
        }
        return m_Indexes[index].appBase + arrIndex;
    }
    return -1;
}

GLint ProgramData::locationWARAppToHost(GLint appLoc)
{
    if (!m_locShiftWAR) return appLoc;

    for(GLuint i=0; i<m_numIndexes; i++) {
        GLint elemIndex = appLoc - m_Indexes[i].appBase;
        if (elemIndex >= 0 && elemIndex < m_Indexes[i].size) {
            return m_Indexes[i].base +
                   elemIndex * m_Indexes[i].hostLocsPerElement;
        }
    }
    return -1;
}

GLint ProgramData::getNextSamplerUniform(GLint index, GLint* val, GLenum* target)
{
    for (GLint i = index + 1; i >= 0 && i < (GLint)m_numIndexes; i++) {
        if (m_Indexes[i].type == GL_SAMPLER_2D) {
            if (val) *val = m_Indexes[i].samplerValue;
            if (target) {
                if (m_Indexes[i].flags & INDEX_FLAG_SAMPLER_EXTERNAL) {
                    *target = GL_TEXTURE_EXTERNAL_OES;
                } else {
                    *target = GL_TEXTURE_2D;
                }
            }
            return i;
        }
    }
    return -1;
}

bool ProgramData::setSamplerUniform(GLint appLoc, GLint val, GLenum* target)
{
    for (GLuint i = 0; i < m_numIndexes; i++) {
        GLint elemIndex = appLoc - m_Indexes[i].appBase;
        if (elemIndex >= 0 && elemIndex < m_Indexes[i].size) {
            if (m_Indexes[i].type == GL_TEXTURE_2D) {
                m_Indexes[i].samplerValue = val;
                if (target) {
                    if (m_Indexes[i].flags & INDEX_FLAG_SAMPLER_EXTERNAL) {
                        *target = GL_TEXTURE_EXTERNAL_OES;
                    } else {
                        *target = GL_TEXTURE_2D;
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool ProgramData::attachShader(GLuint shader)
{
    size_t n = m_shaders.size();
    for (size_t i = 0; i < n; i++) {
        if (m_shaders[i] == shader) {
            return false;
        }
    }
    // AKA m_shaders.push_back(), but that has an ambiguous call to insertAt()
    // due to the default parameters. This is the desired insertAt() overload.
    m_shaders.insertAt(shader, m_shaders.size(), 1);
    return true;
}

bool ProgramData::detachShader(GLuint shader)
{
    size_t n = m_shaders.size();
    for (size_t i = 0; i < n; i++) {
        if (m_shaders[i] == shader) {
            m_shaders.removeAt(i);
            return true;
        }
    }
    return false;
}

/***** GLSharedGroup ****/

GLSharedGroup::GLSharedGroup() :
    m_buffers(android::DefaultKeyedVector<GLuint, BufferData*>(NULL)),
    m_programs(android::DefaultKeyedVector<GLuint, ProgramData*>(NULL)),
    m_shaders(android::DefaultKeyedVector<GLuint, ShaderData*>(NULL))
{
}

GLSharedGroup::~GLSharedGroup()
{
    m_buffers.clear();
    m_programs.clear();
    clearObjectMap(m_buffers);
    clearObjectMap(m_programs);
    clearObjectMap(m_shaders);
}

bool GLSharedGroup::isObject(GLuint obj)
{
    android::AutoMutex _lock(m_lock);
    return ((m_shaders.valueFor(obj)!=NULL) || (m_programs.valueFor(obj)!=NULL));
}

BufferData * GLSharedGroup::getBufferData(GLuint bufferId)
{
    android::AutoMutex _lock(m_lock);
    return m_buffers.valueFor(bufferId);
}

void GLSharedGroup::addBufferData(GLuint bufferId, GLsizeiptr size, void * data)
{
    android::AutoMutex _lock(m_lock);
    m_buffers.add(bufferId, new BufferData(size, data));
}

void GLSharedGroup::updateBufferData(GLuint bufferId, GLsizeiptr size, void * data)
{
    android::AutoMutex _lock(m_lock);
    ssize_t idx = m_buffers.indexOfKey(bufferId);
    if (idx >= 0) {
        delete m_buffers.valueAt(idx);
        m_buffers.editValueAt(idx) = new BufferData(size, data);
    } else {
        m_buffers.add(bufferId, new BufferData(size, data));
    }
}

GLenum GLSharedGroup::subUpdateBufferData(GLuint bufferId, GLintptr offset, GLsizeiptr size, void * data)
{
    android::AutoMutex _lock(m_lock);
    BufferData * buf = m_buffers.valueFor(bufferId);
    if ((!buf) || (buf->m_size < offset+size) || (offset < 0) || (size<0)) return GL_INVALID_VALUE;

    //it's safe to update now
    memcpy((char*)buf->m_fixedBuffer.ptr() + offset, data, size);
    return GL_NO_ERROR;
}

void GLSharedGroup::deleteBufferData(GLuint bufferId)
{
    android::AutoMutex _lock(m_lock);
    ssize_t idx = m_buffers.indexOfKey(bufferId);
    if (idx >= 0) {
        delete m_buffers.valueAt(idx);
        m_buffers.removeItemsAt(idx);
    }
}

void GLSharedGroup::addProgramData(GLuint program)
{
    android::AutoMutex _lock(m_lock);
    ProgramData *pData = m_programs.valueFor(program);
    if (pData)
    {
        m_programs.removeItem(program);
        delete pData;
    }

    m_programs.add(program,new ProgramData());
}

void GLSharedGroup::initProgramData(GLuint program, GLuint numIndexes)
{
    android::AutoMutex _lock(m_lock);
    ProgramData *pData = m_programs.valueFor(program);
    if (pData)
    {
        pData->initProgramData(numIndexes);
    }
}

bool GLSharedGroup::isProgramInitialized(GLuint program)
{
    android::AutoMutex _lock(m_lock);
    ProgramData* pData = m_programs.valueFor(program);
    if (pData)
    {
        return pData->isInitialized();
    }
    return false;
}

void GLSharedGroup::deleteProgramData(GLuint program)
{
    android::AutoMutex _lock(m_lock);
    ProgramData *pData = m_programs.valueFor(program);
    if (pData)
        delete pData;
    m_programs.removeItem(program);
}

void GLSharedGroup::attachShader(GLuint program, GLuint shader)
{
    android::AutoMutex _lock(m_lock);
    ProgramData* programData = m_programs.valueFor(program);
    ssize_t idx = m_shaders.indexOfKey(shader);
    if (programData && idx >= 0) {
        if (programData->attachShader(shader)) {
            refShaderDataLocked(idx);
        }
    }
}

void GLSharedGroup::detachShader(GLuint program, GLuint shader)
{
    android::AutoMutex _lock(m_lock);
    ProgramData* programData = m_programs.valueFor(program);
    ssize_t idx = m_shaders.indexOfKey(shader);
    if (programData && idx >= 0) {
        if (programData->detachShader(shader)) {
            unrefShaderDataLocked(idx);
        }
    }
}

void GLSharedGroup::setProgramIndexInfo(GLuint program, GLuint index, GLint base, GLint size, GLenum type, const char* name)
{
    android::AutoMutex _lock(m_lock);
    ProgramData* pData = m_programs.valueFor(program);
    if (pData)
    {
        pData->setIndexInfo(index,base,size,type);

        if (type == GL_SAMPLER_2D) {
            size_t n = pData->getNumShaders();
            for (size_t i = 0; i < n; i++) {
                GLuint shaderId = pData->getShader(i);
                ShaderData* shader = m_shaders.valueFor(shaderId);
                if (!shader) continue;
                ShaderData::StringList::iterator nameIter = shader->samplerExternalNames.begin();
                ShaderData::StringList::iterator nameEnd  = shader->samplerExternalNames.end();
                while (nameIter != nameEnd) {
                    if (*nameIter == name) {
                        pData->setIndexFlags(index, ProgramData::INDEX_FLAG_SAMPLER_EXTERNAL);
                        break;
                    }
                    ++nameIter;
                }
            }
        }
    }
}

GLenum GLSharedGroup::getProgramUniformType(GLuint program, GLint location)
{
    android::AutoMutex _lock(m_lock);
    ProgramData* pData = m_programs.valueFor(program);
    GLenum type=0;
    if (pData)
    {
        type = pData->getTypeForLocation(location);
    }
    return type;
}

bool  GLSharedGroup::isProgram(GLuint program)
{
    android::AutoMutex _lock(m_lock);
    ProgramData* pData = m_programs.valueFor(program);
    return (pData!=NULL);
}

void GLSharedGroup::setupLocationShiftWAR(GLuint program)
{
    android::AutoMutex _lock(m_lock);
    ProgramData* pData = m_programs.valueFor(program);
    if (pData) pData->setupLocationShiftWAR();
}

GLint GLSharedGroup::locationWARHostToApp(GLuint program, GLint hostLoc, GLint arrIndex)
{
    android::AutoMutex _lock(m_lock);
    ProgramData* pData = m_programs.valueFor(program);
    if (pData) return pData->locationWARHostToApp(hostLoc, arrIndex);
    else return hostLoc;
}

GLint GLSharedGroup::locationWARAppToHost(GLuint program, GLint appLoc)
{
    android::AutoMutex _lock(m_lock);
    ProgramData* pData = m_programs.valueFor(program);
    if (pData) return pData->locationWARAppToHost(appLoc);
    else return appLoc;
}

bool GLSharedGroup::needUniformLocationWAR(GLuint program)
{
    android::AutoMutex _lock(m_lock);
    ProgramData* pData = m_programs.valueFor(program);
    if (pData) return pData->needUniformLocationWAR();
    return false;
}

GLint GLSharedGroup::getNextSamplerUniform(GLuint program, GLint index, GLint* val, GLenum* target) const
{
    android::AutoMutex _lock(m_lock);
    ProgramData* pData = m_programs.valueFor(program);
    return pData ? pData->getNextSamplerUniform(index, val, target) : -1;
}

bool GLSharedGroup::setSamplerUniform(GLuint program, GLint appLoc, GLint val, GLenum* target)
{
    android::AutoMutex _lock(m_lock);
    ProgramData* pData = m_programs.valueFor(program);
    return pData ? pData->setSamplerUniform(appLoc, val, target) : false;
}

bool GLSharedGroup::addShaderData(GLuint shader)
{
    android::AutoMutex _lock(m_lock);
    ShaderData* data = new ShaderData;
    if (data) {
        if (m_shaders.add(shader, data) < 0) {
            delete data;
            data = NULL;
        }
        data->refcount = 1;
    }
    return data != NULL;
}

ShaderData* GLSharedGroup::getShaderData(GLuint shader)
{
    android::AutoMutex _lock(m_lock);
    return m_shaders.valueFor(shader);
}

void GLSharedGroup::unrefShaderData(GLuint shader)
{
    android::AutoMutex _lock(m_lock);
    ssize_t idx = m_shaders.indexOfKey(shader);
    if (idx >= 0) {
        unrefShaderDataLocked(idx);
    }
}

void GLSharedGroup::refShaderDataLocked(ssize_t shaderIdx)
{
    assert(shaderIdx >= 0 && shaderIdx <= m_shaders.size());
    ShaderData* data = m_shaders.valueAt(shaderIdx);
    data->refcount++;
}

void GLSharedGroup::unrefShaderDataLocked(ssize_t shaderIdx)
{
    assert(shaderIdx >= 0 && shaderIdx <= m_shaders.size());
    ShaderData* data = m_shaders.valueAt(shaderIdx);
    if (--data->refcount == 0) {
        delete data;
        m_shaders.removeItemsAt(shaderIdx);
    }
}
