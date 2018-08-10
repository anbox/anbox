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
#include "EntryPoint.h"

#include "Parser.h"
#include "TypeFactory.h"
#include "strUtils.h"

#include <sstream>
#include <string>

#include <stdio.h>

EntryPoint::EntryPoint()
{
    reset();
}

EntryPoint::~EntryPoint()
{
}

void EntryPoint::reset()
{
    m_unsupported = false;
    m_customDecoder = false;
    m_notApi = false;
    m_flushOnEncode = false;
    m_vars.empty();
}

// return true for valid line (need to get into the entry points list)
bool EntryPoint::parse(unsigned int lc, const std::string & str)
{
    size_t pos, last;
    std::string field;

    reset();
    std::string linestr = trim(str);

    if (linestr.size() == 0) return false;
    if (linestr.at(0) == '#') return false;

    // skip PREFIX
    field = getNextToken(linestr, 0, &last, "(");
    pos = last + 1;
    // return type
    field = getNextToken(linestr, pos, &last, ",)");

    std::string error;
    std::string retTypeName;
    if (!parseTypeDeclaration(field, &retTypeName, &error)) {
        fprintf(stderr,
                "line: %d: Parsing error in field <%s>: %s\n",
                lc, 
                field.c_str(), 
                error.c_str());
        return false;
    }
    pos = last + 1;
    const VarType *theType = TypeFactory::instance()->getVarTypeByName(retTypeName);
    if (theType->name() == "UNKNOWN") {
        fprintf(stderr, "UNKNOWN retval: %s\n", linestr.c_str());
    }

    m_retval.init(std::string(""),
                  theType,
                  std::string(""),
                  Var::POINTER_OUT,
                  std::string(""),
                  std::string(""));

    // function name
    m_name = getNextToken(linestr, pos, &last, ",)");
    pos = last + 1;

    // parameters;
    int nvars = 0;
    while (pos < linestr.size() - 1) {
        field = getNextToken(linestr, pos, &last, ",)");
        if (field == "void") {
            // 'void' is used as a special case for functions that don't take
            // parameters at all.
            break;
        }
        std::string vartype, varname;
        if (!parseParameterDeclaration(field, &vartype, &varname, &error)) {
            fprintf(stderr,
                    "line: %d: Parsing error in field <%s>, error: %s\n",
                    lc,
                    field.c_str(),
                    error.c_str());
            return false;
        }
        nvars++;
        const VarType *v = TypeFactory::instance()->getVarTypeByName(vartype);
        if (v->id() == 0) {
            fprintf(stderr, "%d: Unknown type: %s\n", lc, vartype.c_str());
        } else {
            if (varname == "" &&
                !(v->name() == "void" && !v->isPointer())) {
                std::ostringstream oss;
                oss << "var" << nvars;
                varname = oss.str();
            }

            m_vars.push_back(Var(varname, v, std::string(""), Var::POINTER_IN, "", ""));
        }
        pos = last + 1;
    }
    return true;
}

void EntryPoint::print(FILE *fp, bool newline,
                       const std::string & name_suffix,
                       const std::string & name_prefix,
                       const std::string & ctx_param ) const
{
    fprintf(fp, "%s %s%s%s(",
            m_retval.type()->name().c_str(),
            name_prefix.c_str(),
            m_name.c_str(),
            name_suffix.c_str());

    if (ctx_param != "") fprintf(fp, "%s ", ctx_param.c_str());

    for (size_t i = 0; i < m_vars.size(); i++) {
        if (m_vars[i].isVoid()) continue;
        if (i != 0 || ctx_param != "") fprintf(fp, ", ");
        fprintf(fp, "%s %s", m_vars[i].type()->name().c_str(),
                m_vars[i].name().c_str());
    }
    fprintf(fp, ")%s", newline? "\n" : "");
}

Var * EntryPoint::var(const std::string & name)
{
    Var *v = NULL;
    for (size_t i = 0; i < m_vars.size(); i++) {
        if (m_vars[i].name() == name) {
            v = &m_vars[i];
            break;
        }
    }
    return v;
}

bool EntryPoint::hasPointers()
{
    bool pointers = false;
    if (m_retval.isPointer()) pointers = true;
    if (!pointers) {
        for (size_t i = 0; i < m_vars.size(); i++) {
            if (m_vars[i].isPointer()) {
                pointers = true;
                break;
            }
        }
    }
    return pointers;
}

int EntryPoint::setAttribute(const std::string &line, size_t lc)
{
    size_t pos = 0;
    size_t last;
    std::string token = getNextToken(line, 0, &last, WHITESPACE);

    if (token == "len") {
        pos = last;
        std::string varname = getNextToken(line, pos, &last, WHITESPACE);

        if (varname.size() == 0) {
            fprintf(stderr, "ERROR: %u: Missing variable name in 'len' attribute\n", (unsigned int)lc);
            return -1;
        }
        Var * v = var(varname);
        if (v == NULL) {
            fprintf(stderr, "ERROR: %u: variable %s is not a parameter of %s\n",
                    (unsigned int)lc, varname.c_str(), name().c_str());
            return -2;
        }
        // set the size expression into var
        pos = last;
        v->setLenExpression(line.substr(pos));
    } else if (token == "param_check") {
        pos = last;
        std::string varname = getNextToken(line, pos, &last, WHITESPACE);

        if (varname.size() == 0) {
            fprintf(stderr, "ERROR: %u: Missing variable name in 'param_check' attribute\n", (unsigned int)lc);
            return -1;
        }
        Var * v = var(varname);
        if (v == NULL) {
            fprintf(stderr, "ERROR: %u: variable %s is not a parameter of %s\n",
                    (unsigned int)lc, varname.c_str(), name().c_str());
            return -2;
        }
        // set the size expression into var
        pos = last;
        v->setParamCheckExpression(line.substr(pos));

    } else if (token == "dir") {
        pos = last;
        std::string varname = getNextToken(line, pos, &last, WHITESPACE);
        if (varname.size() == 0) {
            fprintf(stderr, "ERROR: %u: Missing variable name in 'dir' attribute\n", (unsigned int)lc);
            return -1;
        }
        Var * v = var(varname);
        if (v == NULL) {
            fprintf(stderr, "ERROR: %u: variable %s is not a parameter of %s\n",
                    (unsigned int)lc, varname.c_str(), name().c_str());
            return -2;
        }

        pos = last;
        std::string pointerDirStr = getNextToken(line, pos, &last, WHITESPACE);
        if (pointerDirStr.size() == 0) {
            fprintf(stderr, "ERROR: %u: missing pointer directions\n", (unsigned int)lc);
            return -3;
        }

        if (pointerDirStr == "out") {
            v->setPointerDir(Var::POINTER_OUT);
        } else if (pointerDirStr == "inout") {
            v->setPointerDir(Var::POINTER_INOUT);
        } else if (pointerDirStr == "in") {
            v->setPointerDir(Var::POINTER_IN);
        } else {
            fprintf(stderr, "ERROR: %u: unknow pointer direction %s\n", (unsigned int)lc, pointerDirStr.c_str());
        }
    } else if (token == "var_flag") {
        pos = last;
        std::string varname = getNextToken(line, pos, &last, WHITESPACE);
        if (varname.size() == 0) {
            fprintf(stderr, "ERROR: %u: Missing variable name in 'var_flag' attribute\n", (unsigned int)lc);
            return -1;
        }
        Var * v = var(varname);
        if (v == NULL) {
            fprintf(stderr, "ERROR: %u: variable %s is not a parameter of %s\n",
                    (unsigned int)lc, varname.c_str(), name().c_str());
            return -2;
        }
        int count = 0;
        for (;;) {
            pos = last;
            std::string flag = getNextToken(line, pos, &last, WHITESPACE);
            if (flag.size() == 0) {
                if (count == 0) {
                    fprintf(stderr, "ERROR: %u: missing flag\n", (unsigned int) lc);
                    return -3;
                }
                break;
            }
            count++;

            if (flag == "nullAllowed") {
                if (v->isPointer()) {
                    v->setNullAllowed(true);
                } else {
                    fprintf(stderr, "WARNING: %u: setting nullAllowed for non-pointer variable %s\n",
                            (unsigned int) lc, v->name().c_str());
                }
            } else if (flag == "isLarge") {
                if (v->isPointer()) {
                    v->setIsLarge(true);
                } else {
                    fprintf(stderr, "WARNING: %u: setting isLarge flag for a non-pointer variable %s\n",
                            (unsigned int) lc, v->name().c_str());
                }
            } else {
                fprintf(stderr, "WARNING: %u: unknow flag %s\n", (unsigned int)lc, flag.c_str());
            }
        }
    } else if (token == "custom_pack") {
        pos = last;
        std::string varname = getNextToken(line, pos, &last, WHITESPACE);

        if (varname.size() == 0) {
            fprintf(stderr, "ERROR: %u: Missing variable name in 'custom_pack' attribute\n", (unsigned int)lc);
            return -1;
        }
        Var * v = var(varname);
        if (v == NULL) {
            fprintf(stderr, "ERROR: %u: variable %s is not a parameter of %s\n",
                    (unsigned int)lc, varname.c_str(), name().c_str());
            return -2;
        }
        // set the size expression into var
        pos = last;
        v->setPackExpression(line.substr(pos));
    } else if (token == "custom_write") {
        pos = last;
        std::string varname = getNextToken(line, pos, &last, WHITESPACE);

        if (varname.size() == 0) {
            fprintf(stderr, "ERROR: %u: Missing variable name in 'custom_write' attribute\n", (unsigned int)lc);
            return -1;
        }
        Var * v = var(varname);
        if (v == NULL) {
            fprintf(stderr, "ERROR: %u: variable %s is not a parameter of %s\n",
                    (unsigned int)lc, varname.c_str(), name().c_str());
            return -2;
        }
        // set the size expression into var
        pos = last;
        v->setWriteExpression(line.substr(pos));
    } else if (token == "flag") {
        pos = last;
        std::string flag = getNextToken(line, pos, &last, WHITESPACE);
        if (flag.size() == 0) {
            fprintf(stderr, "ERROR: %u: missing flag\n", (unsigned int) lc);
            return -4;
        }

        if (flag == "unsupported") {
            setUnsupported(true);
        } else if (flag == "custom_decoder") {
            setCustomDecoder(true);
        } else if (flag == "not_api") {
            setNotApi(true);
        } else if (flag == "flushOnEncode") {
            setFlushOnEncode(true);
        } else {
            fprintf(stderr, "WARNING: %u: unknown flag %s\n", (unsigned int)lc, flag.c_str());
        }
    } else {
        fprintf(stderr, "WARNING: %u: unknown attribute %s\n", (unsigned int)lc, token.c_str());
    }

    return 0;
}
