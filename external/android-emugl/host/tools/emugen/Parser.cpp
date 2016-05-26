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

#include <vector>

#define WHITESPACE " \t\n"

// Parse the |input| string as a list of type-specific tokens.
// This tokenizes the input, using whitespace as separators and '*' as
// a single token too. On success, return true and sets |*out| to the
// list of tokens. On failure, return false.
//
// Example: 'const char**foo' -> ['const', 'char', '*', '*', 'foo']
//
static bool parseTypeTokens(const std::string& input,
                            std::vector<std::string>* out,
                            std::string* error) {
    out->clear();
    size_t pos = 0U;

    // Parse all tokens in the input, treat '*' as a single token.
    // I.e.
    for (;;) {
        // skip leading whitespace.
        pos = input.find_first_not_of(WHITESPACE, pos);
        if (pos == std::string::npos) {
            break;  // end of parse.
        }

        // If this is a star, ensure it follows a type name.
        // otherwise treat it as part of the final type.
        if (input[pos] == '*') {
            out->push_back(std::string("*"));
            pos += 1U;
            continue;
        }

        // find end of type/token.
        size_t end = input.find_first_of(WHITESPACE "*", pos);
        if (end == std::string::npos) {
            end = input.size();
        }

        std::string str = input.substr(pos, end - pos);
        if (str.size() == 0) {
            // Sanity check: should not happen.
            if (error != NULL) {
                *error = "Unexpected empty token !?";
            }
            return false;
        }

        out->push_back(str);
        pos = end;
    }

    if (error != NULL) {
        // Sanity check: require non-empty input
        if (out->empty()) {
            *error = "Empty parameter declaration!";
            return false;
        }

        // Sanity check: There must be base type name before any '*'
        for (size_t n = 0; n < out->size(); ++n) {
            std::string& token = (*out)[n];
            if (token == "*") {
                *error = "Unexpected '*' before type name";
                return false;
            } else if (token != "const") {
                break;
            }
        }
    }

    return true;
}

// Given |tokens|, an input vector of strings, join the first |count| items
// into a normalized type string, and return it.
static std::string buildTypeString(const std::vector<std::string>& tokens,
                                   size_t count) {
    std::string result;

    for (size_t n = 0; n < count; ++n) {
        const std::string& token = tokens[n];
        if (n > 0 && token != "*") {
            result.append(" ");
        }
        result.append(token);
    }
    return result;
}


std::string normalizeTypeDeclaration(const std::string& input) {
    std::vector<std::string> tokens;
    if (!parseTypeTokens(input, &tokens, NULL)) {
        return "";
    }
    return buildTypeString(tokens, tokens.size());
}

bool parseTypeDeclaration(const std::string& input,
                          std::string* typeName,
                          std::string* error) {
    // The type name can be made of several tokens, e.g. 'unsigned int'
    // use an array to store them, and a count variable. Each item can be
    // one of '*', 'const' or a type name component (e.g. 'struct', 'unsigned')
    std::vector<std::string> tokens;

    if (!parseTypeTokens(input, &tokens, error)) {
        return false;
    }

    // Sanity check, there must be a least one non-special tokens.
    size_t nonSpecialCount = 0;
    for (size_t n = 0; n < tokens.size(); ++n) {
        if (tokens[n] != "*" && tokens[n] != "const") {
            nonSpecialCount++;
        }
    }
    if (nonSpecialCount == 0) {
        *error = "Missing type name";
        return false;
    }
    // Build the type name from all tokens before it.
    *typeName = buildTypeString(tokens, tokens.size());
    return true;
}


bool parseParameterDeclaration(const std::string& param,
                               std::string* typeName,
                               std::string* variableName,
                               std::string* error) {
    std::vector<std::string> tokens;

    if (!parseTypeTokens(param, &tokens, error)) {
        return false;
    }

    // Sanity check, there must be a least two non-special tokens.
    size_t nonSpecialCount = 0;
    for (size_t n = 0; n < tokens.size(); ++n) {
        if (tokens[n] != "*" && tokens[n] != "const") {
            nonSpecialCount++;
        }
    }
    if (nonSpecialCount == 0) {
        *error = "Missing type name";
        return false;
    }
    if (nonSpecialCount == 1) {
        *error = "Missing variable name";
        return false;
    }

    // Sanity check: variable name must not be followed by 'const' or '*'
    const std::string& lastToken = tokens[tokens.size() - 1U];
    if (lastToken == "*") {
        *error = "Extra '*' after variable name";
        return false;
    }
    if (lastToken == "const") {
        *error = "Extra 'const' after variable name";
        return false;
    }

    // Extract the variable name as the last token.
    if (variableName) {
        *variableName = lastToken;
    }
    // Build the type name from all tokens before it.
    *typeName = buildTypeString(tokens, tokens.size() - 1U);
    return true;
}
