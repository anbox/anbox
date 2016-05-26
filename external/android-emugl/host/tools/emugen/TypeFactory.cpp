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
#include "TypeFactory.h"

#include "Parser.h"
#include "VarType.h"
#include "strUtils.h"

#include <map>
#include <string>

#include <stdio.h>
#include <stdlib.h>


TypeFactory * TypeFactory::m_instance = NULL;

typedef std::map<std::string, VarType> TypeMap;
static  TypeMap g_varMap;
static bool g_initialized = false;
static int g_typeId = 0;


#define ADD_TYPE(name, size, printformat,ispointer)                                           \
    g_varMap.insert(std::pair<std::string, VarType>(name, VarType(g_typeId++, name, (size + 7) >> 3, printformat , ispointer)));

void TypeFactory::initBaseTypes()
{
    g_initialized = true;
    ADD_TYPE("UNKNOWN", 0, "0x%x", false);
    ADD_TYPE("void", 0, "0x%x", false);
    ADD_TYPE("char", 8, "%c", false);
    ADD_TYPE("int", 32, "%d", false);
    ADD_TYPE("float", 32, "%d", false);
    ADD_TYPE("short", 16, "%d", false);
}

int TypeFactory::initFromFile(const std::string &filename)
{
    if (!g_initialized) {
        initBaseTypes();
    }

    FILE *fp = fopen(filename.c_str(), "rt");
    if (fp == NULL) {
        perror(filename.c_str());
        return -1;
    }
    char line[1000];
    int lc = 0;
    while(fgets(line, sizeof(line), fp) != NULL) {
        lc++;
        std::string str = trim(line);
        if (str.size() == 0 || str.at(0) == '#') {
            continue;
        }
        size_t pos = 0, last;
        std::string name;
        name = getNextToken(str, pos, &last, WHITESPACE);
        name = normalizeTypeDeclaration(name);
        if (name.size() == 0) {
            fprintf(stderr, "Error: %d : missing type name\n", lc);
            return -2;
        }
        pos = last + 1;
        std::string size;
        size = getNextToken(str, pos, &last, WHITESPACE);
        if (size.size() == 0) {
            fprintf(stderr, "Error: %d : missing type width\n", lc);
            return -2;
        }
        pos = last + 1;
        std::string printString;
        printString = getNextToken(str, pos, &last, WHITESPACE);
        if (printString.size() == 0) {
            fprintf(stderr, "Error: %d : missing print-string\n", lc);
            return -2;
        }

        // The ispointer definition is optional since we can just
        // look at the type name, and determine it is a pointer if
        // it ends with '*'.
        bool isPointer = (name[name.size() - 1U] == '*');

        pos = last + 1;
        std::string pointerDef;
        pointerDef = getNextToken(str, pos, &last, WHITESPACE);
        if (pointerDef.size() != 0) {
            // Just a little sanity check.
            if (std::string("true")==pointerDef) {
                if (!isPointer) {
                    fprintf(stderr, "Error: %d: invalid isPointer definition: 'true' but name does not end with '*'!\n", lc);
                    return -2;
                }
            } else if (std::string("false")==pointerDef) {
                if (isPointer) {
                    fprintf(stderr, "Error: %d: invalid isPointer definition: 'false' but name does end with '*'!\n", lc);
                    return -2;
                }
            } else {
                fprintf(stderr, "Error: %d : invalid isPointer definition, must be either \"true\" or \"false\"\n", lc);
                return -2;
            }
        }

        size_t bitSize = atoi(size.c_str());
        size_t byteSize = (bitSize + 7) >> 3;

        if (getVarTypeByName(name)->id() != 0) {
            fprintf(stderr,
                    "Warining: %d : type %s is already known, definition in line %d is taken\n",
                    lc, name.c_str(), lc);
        }
        g_varMap.insert(std::pair<std::string, VarType>(
                name, VarType(g_typeId++,
                              name,
                              byteSize,
                              printString,
                              isPointer)));
        std::string constName = "const " + name;
        g_varMap.insert(std::pair<std::string, VarType>(
                constName, VarType(g_typeId++,
                                   constName,
                                   byteSize,
                                   printString,
                                   isPointer))); //add a const type
    }
    g_initialized = true;
    return 0;
}


const VarType * TypeFactory::getVarTypeByName(const std::string & type)
{
    if (!g_initialized) {
        initBaseTypes();
    }
    TypeMap::iterator i = g_varMap.find(type);
    if (i == g_varMap.end()) {
        i = g_varMap.find("UNKNOWN");
    }
    return &(i->second);
}

