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
#ifndef __VARTYPE__H__
#define __VARTYPE__H__

#include <string>

// VarType models the types of values used on the wire protocol by
// both encoders and decoders. Each type is identified by a unique id,
// and a name, and provides a size in bytes for the values, a printf-like
// formatter string, and a flag telling if the value corresponds to a
// pointer.
class VarType {
public:
    VarType() :
            m_id(0),
            m_name("default_constructed"),
            m_byteSize(0),
            m_printFormat("0x%x"),
            m_isPointer(false) {}

    VarType(size_t id,
            const std::string& name,
            size_t byteSize,
            const std::string& printFormat,
            bool isPointer) :
            m_id(id),
            m_name(name),
            m_byteSize(byteSize),
            m_printFormat(printFormat),
            m_isPointer(isPointer) {}

    ~VarType() {}

    size_t id() const { return m_id; }
    const std::string& name() const { return m_name; }
    size_t bytes() const { return m_byteSize; }
    const std::string& printFormat() const { return m_printFormat; }
    bool isPointer() const { return m_isPointer; }

private:
    size_t m_id;
    std::string m_name;
    size_t m_byteSize;
    std::string m_printFormat;
    bool m_isPointer;
};

#endif
