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
#ifndef PARSER_H
#define PARSER_H

#include <string>

// Normalize a type declaration. This gets rid of leading/trailing whitespace
// as well as ensure there is a single space separating all input tokens,
// with the exception of '*' which is treated specially by never being
// prepended with a space.
// |input| is the input type declaration. Return normalized form.
// Note that this doesn't try to validate the input.
std::string normalizeTypeDeclaration(const std::string& input);

// Parse a return type declaration. |input| is the type declaration
// (e.g. 'const char**'). On success return true and sets |*typeName| to
// the appropriate normalized type string. On failure, return false and
// sets |*error| with a human-friendly message explaining the reason for
// failure.
//
// Note that the returned type string is normalized, see comment for
// parseParameterDeclaration() for examples.
//
// NOTE: This does not support declarations of arrays or functions!
//
bool parseTypeDeclaration(const std::string& input,
                          std::string* typeName,
                          std::string* error);

// Parse a function parameter declaration and extract the type and variable
// name from it. |param| is the individual parameter declaration from the
// function's signature (e.g. 'const char *items')
//
// On success, returns true and sets |*vartype| and |*varname| to the
// appropriate type name and variable name. |varname| can be NULL if the caller
// isn't interested in the variable name.
//
// On failure, return false and sets |*error| to a human-friendly message
// explaining the reason for failure.
//
// Note that the returned type name is normalized with regards to whitespace
// and star location, e.g.:
//
//      const void *items           -> 'const void*'
//      char* *items                -> 'char**'
//      const void * const * items  -> 'const void* const*'
//      unsigned int *const data    -> 'unsigned int* const'
//
bool parseParameterDeclaration(const std::string& param,
                               std::string* typeName,
                               std::string* variableName,
                               std::string* error);

#endif  // PARSER_H
