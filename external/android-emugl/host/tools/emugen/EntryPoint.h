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
#ifndef __EntryPoint__H__
#define __EntryPoint__H__

#include <string>
#include <vector>
#include <stdio.h>

#include "Var.h"

//---------------------------------------------------

typedef std::vector<Var> VarsArray;

class EntryPoint {
public:
    EntryPoint();
    virtual ~EntryPoint();
    bool parse(unsigned int lc, const std::string & str);
    void reset(); // reset the class to empty;
    void print(FILE *fp = stdout, bool newline = true,
               const std::string & name_suffix = std::string(""),
               const std::string & name_prefix = std::string(""),
               const std::string & ctx_param = std::string("")) const;
    const std::string & name() const { return m_name; }
    VarsArray & vars() { return m_vars; }
    Var & retval() { return m_retval; }
    Var * var(const std::string & name);
    bool hasPointers();
    bool unsupported() const { return m_unsupported; }
    void setUnsupported(bool state) { m_unsupported = state; }
    bool customDecoder() { return m_customDecoder; }
    void setCustomDecoder(bool state) { m_customDecoder = state; }
    bool notApi() const { return m_notApi; }
    void setNotApi(bool state) { m_notApi = state; }
    bool flushOnEncode() const { return m_flushOnEncode; }
    void setFlushOnEncode(bool state) { m_flushOnEncode = state; }
    int setAttribute(const std::string &line, size_t lc);

private:
    enum { PR_RETVAL = 0, PR_NAME, PR_VARS, PR_DONE } prState;
    std::string m_name;
    Var m_retval;
    VarsArray m_vars;
    bool m_unsupported;
    bool m_customDecoder;
    bool m_notApi;
    bool m_flushOnEncode;

    void err(unsigned int lc, const char *msg) {
        fprintf(stderr, "line %d: %s\n", lc, msg);
    }
};


#endif
