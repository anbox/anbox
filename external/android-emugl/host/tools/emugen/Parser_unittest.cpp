/*
* Copyright 2014 The Android Open Source Project
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
#include "Parser.h"

#include <gtest/gtest.h>

#define ARRAYLEN(x)  (sizeof(x) / sizeof(x[0]))

TEST(ParserTest, normalizeTypeDeclaration) {
    static const struct {
        const char* expected;
        const char* input;
    } kData[] = {
        { "char", "char" },
        { "const unsigned int", "   const   unsigned\tint\n" },
        { "char* const**", "char *const* *" },
    };
    const size_t kDataSize = ARRAYLEN(kData);
    for (size_t n = 0; n < kDataSize; ++n) {
        std::string result;
        std::string text = "When parsing '";
        text += kData[n].input;
        text += "'";

        result = normalizeTypeDeclaration(kData[n].input);
        EXPECT_STREQ(kData[n].expected, result.c_str()) << text;
    }
}

TEST(ParserTest, parseTypeDeclaration) {
    static const struct {
        const char* input;
        bool expected;
        const char* expectedType;
        const char* expectedError;
    } kData[] = {
        { "const", false, NULL, "Missing type name" },
        { "const const", false, NULL, "Missing type name" },
        { "foo", true, "foo", NULL },
        { "void", true, "void", NULL },
        { "const foo", true, "const foo", NULL },
        { "foo *", true, "foo*", NULL },
        { "char foo", true, "char foo", NULL },
        { "\tunsigned \t  int\n", true, "unsigned int", NULL },
        { "const * char", false, NULL, "Unexpected '*' before type name" },
        { "const char * ", true, "const char*", NULL },
        { "const void*const * *", true, "const void* const**", NULL },
    };
    const size_t kDataSize = ARRAYLEN(kData);
    for (size_t n = 0; n < kDataSize; ++n) {
        std::string varname, vartype, error;
        std::string text = "When parsing '";
        text += kData[n].input;
        text += "'";

        EXPECT_EQ(kData[n].expected,
                  parseTypeDeclaration(kData[n].input,
                                       &vartype,
                                       &error)) << text;
        if (kData[n].expected) {
            EXPECT_STREQ(kData[n].expectedType, vartype.c_str()) << text;
        } else {
            EXPECT_STREQ(kData[n].expectedError, error.c_str()) << text;
        }
    }
}

TEST(ParserTest, parseParameterDeclaration) {
    static const struct {
        const char* input;
        bool expected;
        const char* expectedType;
        const char* expectedVariable;
        const char* expectedError;
    } kData[] = {
        { "foo", false, NULL, NULL, "Missing variable name" },
        { "const", false, NULL, NULL, "Missing type name" },
        { "const foo", false, NULL, NULL, "Missing variable name" },
        { "const const", false, NULL, NULL, "Missing type name" },
        { "char foo", true, "char", "foo", NULL },
        { "unsigned   int\t bar\n", true, "unsigned int", "bar", NULL },
        { "const * char foo", false, NULL, NULL, "Unexpected '*' before type name" },
        { "const char * foo", true, "const char*", "foo", NULL },
        { "const void*const *data", true, "const void* const*", "data", NULL },
        { "char foo const", false, NULL, NULL, "Extra 'const' after variable name" },
        { "int bar*", false, NULL, NULL, "Extra '*' after variable name" },
    };
    const size_t kDataSize = ARRAYLEN(kData);
    for (size_t n = 0; n < kDataSize; ++n) {
        std::string varname, vartype, error;
        std::string text = "When parsing '";
        text += kData[n].input;
        text += "'";

        EXPECT_EQ(kData[n].expected,
                  parseParameterDeclaration(kData[n].input,
                                            &vartype,
                                            &varname,
                                            &error)) << text;
        if (kData[n].expected) {
            EXPECT_STREQ(kData[n].expectedType, vartype.c_str()) << text;
            EXPECT_STREQ(kData[n].expectedVariable, varname.c_str()) << text;
        } else {
            EXPECT_STREQ(kData[n].expectedError, error.c_str()) << text;
        }
    }
}
