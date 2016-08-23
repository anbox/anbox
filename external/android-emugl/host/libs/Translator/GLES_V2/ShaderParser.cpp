/*
* Copyright 2011 The Android Open Source Project
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

#include "ShaderParser.h"
#include <stdlib.h>
#include <string.h>

ShaderParser::ShaderParser():ObjectData(SHADER_DATA),
                             m_type(0),
                             m_originalSrc(NULL),
                             m_parsedLines(NULL),
                             m_deleteStatus(false),
                             m_program(0) {
    m_infoLog = new GLchar[1];
    m_infoLog[0] = '\0';
};

ShaderParser::ShaderParser(GLenum type):ObjectData(SHADER_DATA), 
                                        m_type(type),
                                        m_originalSrc(NULL),
                                        m_parsedLines(NULL),
                                        m_deleteStatus(false),
                                        m_program(0) {

    m_infoLog = new GLchar[1];
    m_infoLog[0] = '\0';
};

void ShaderParser::setSrc(const Version& ver,GLsizei count,const GLchar* const* strings,const GLint* length){
    m_src.clear();
    for(int i = 0;i<count;i++){
        m_src.append(strings[i]);
    }
    //store original source
    if (m_originalSrc)
        free(m_originalSrc);
    m_originalSrc = strdup(m_src.c_str());

    clearParsedSrc();

    // parseGLSLversion must be called first since #version should be the
    // first token in the shader source.
    parseGLSLversion();
    parseBuiltinConstants();
    /*
      version 1.30.10 is the first version of GLSL Language containing precision qualifiers
      if the glsl version is less than 1.30.10 than we will use a shader parser which omits
      all precision qualifiers from the shader source , otherwise we will use a shader parser
      which set the default precisions to be the same as the default precisions of GLSL ES
    */
#if 0
    if(ver < Version(1,30,10)){
        parseOmitPrecision();
     } else {
        parseExtendDefaultPrecision();
     }
#else
    //XXX: Until proved otherwise, glsl doesn't know/use those precision macros, so we omit then
    parseOmitPrecision();
#endif
    parseLineNumbers();
    parseOriginalSrc();
}
const GLchar** ShaderParser::parsedLines() {
      m_parsedLines = (GLchar*)m_parsedSrc.c_str();
      return const_cast<const GLchar**> (&m_parsedLines);
};

const char* ShaderParser::getOriginalSrc(){
    return m_originalSrc;
}

void ShaderParser::parseLineNumbers()
{
    m_parsedSrc += "#line 1\n";
}

void ShaderParser::parseOriginalSrc() {
    m_parsedSrc+=m_src;
}

void ShaderParser::parseGLSLversion() {

    //
    // find in shader the #version token if exist.
    // That token should be the first non-comment or blank token
    //
    const char *src = m_src.c_str();
    const int minGLSLVersion = 120;
    int glslVersion = minGLSLVersion;
    enum {
        PARSE_NONE,
        PARSE_IN_C_COMMENT,
        PARSE_IN_LINE_COMMENT
    } parseState = PARSE_NONE;
    const char *c = src;

    #define IS_VALID_VERSION(v) ( (v) == 100 || (v) == 300 || (v) == 310 )

    while( c && *c != '\0') {
        if (parseState == PARSE_IN_C_COMMENT) {
            if (*c == '*' && *(c+1) == '/') {
                parseState = PARSE_NONE;
                c += 2;
            }
            else c++;
        }
        else if (parseState == PARSE_IN_LINE_COMMENT) {
            if (*c == '\n') {
                parseState = PARSE_NONE;
            }
            c++;
        }
        else if (*c == '/' && *(c+1) == '/') {
            parseState = PARSE_IN_LINE_COMMENT;
            c += 2;
        }
        else if (*c == '/' && *(c+1) == '*') {
            parseState = PARSE_IN_C_COMMENT;
            c += 2;
        }
        else if (*c == ' ' || *c == '\t' || *c == '\r' || *c == '\n') {
            c++;
        }
        else {
            //
            // We have reached the first non-blank character outside
            // a comment, this must be a #version token or else #version
            // token does not exist in this shader source.
            //
            if (!strncmp(c,"#version",8)) {
                int ver;
                if (sscanf(c+8,"%d",&ver) == 1) {
                    //
                    // parsed version string correctly, blank out the
                    // version token from the source, we will add it later at
                    // the begining of the shader.
                    //
                    char *cc = (char *)c;
                    for (int i=0; i<8; i++,cc++) *cc = ' ';
                    while (*cc < '0' || *cc > '9') { *cc = ' '; cc++; }
                    while (*cc >= '0' && *cc <= '9') { *cc = ' '; cc++; }

                    // Use the version from the source but only if
                    // it is larger than our minGLSLVersion
                    if (!IS_VALID_VERSION(ver) || ver > minGLSLVersion) glslVersion = ver;
                }
            }

            //
            // break the loop, no need to go further on the source.
            break;
        }
    }

    //
    // allow to force GLSL version through environment variable
    //
    const char *forceVersion = getenv("GOOGLE_GLES_FORCE_GLSL_VERSION");
    if (forceVersion) {
        int ver;
        if (sscanf(forceVersion,"%d",&ver) == 1) {
            glslVersion = ver;
        }
    }

    //
    // if glslVersion is defined, add it to the parsed source
    //
    if (glslVersion > 0) {
        char vstr[16];
        sprintf(vstr,"%d",glslVersion);
        m_parsedSrc += std::string("#version ") + 
                       std::string(vstr) + 
                       std::string("\n");
    }
}

void ShaderParser::parseBuiltinConstants()
{
    m_parsedSrc += 
                   "const int _translator_gl_MaxVertexUniformVectors = 256;\n"
                   "const int _translator_gl_MaxFragmentUniformVectors = 256;\n"
                   "const int _translator_gl_MaxVaryingVectors = 15;\n"
                   "#define gl_MaxVertexUniformVectors _translator_gl_MaxVertexUniformVectors\n"
                   "#define gl_MaxFragmentUniformVectors _translator_gl_MaxFragmentUniformVectors\n"
                   "#define gl_MaxVaryingVectors _translator_gl_MaxVaryingVectors\n";

}

void ShaderParser::parseOmitPrecision(){

    //defines we need to add in order to Omit precisions qualifiers
    static const GLchar defines[] = {
                                         "#define GLES 1\n"
                                         "#define lowp \n"
                                         "#define mediump \n"
                                         "#define highp \n"
                                     };
    m_parsedSrc+=defines;

    //
    // parse the source and blank out precision statements
    // which has the following syntax:
    //   precision {qualifier} {type};
    // where {qualifier} is one of lowp,mediump or hightp
    // type is any valid GLES defined type (we do not check that here!)
    // NOTE: This is needed in order to workaround driver bug in
    //       Intel/Linux where the compiler does not get statement like
    //       "float;", otherwise we could just define a macro named
    //       precision to be empty.
    //
    const char *src = m_src.c_str();

    enum { 
        PRECISION,
        QUALIFIER,
        SEMICOLON
    } statementState = PRECISION;
    const char *precision = NULL;

    enum {
        PARSE_NONE,
        PARSE_IN_C_COMMENT,
        PARSE_IN_LINE_COMMENT
    } parseState = PARSE_NONE;
    const char *c = src;
    const char *t = NULL;

    #define IS_DELIMITER(c) ( (c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n' )
    #define IS_TOKEN_START(c) ( ((c) >= 'a' && (c) <='z') || ((c) >= 'A' && (c) <= 'Z') )
    #define IS_TOKEN_DELIMITER(c) ( IS_DELIMITER(c) || (c) == ';' )

    while( c && *c != '\0') {
        if (parseState == PARSE_IN_C_COMMENT) {
            if (*c == '*' && *(c+1) == '/') {
                parseState = PARSE_NONE;
                c += 2;
            }
            else c++;
        }
        else if (parseState == PARSE_IN_LINE_COMMENT) {
            if (*c == '\n') {
                parseState = PARSE_NONE;
            }
            c++;
        }
        else if (*c == '/' && *(c+1) == '/') {
            parseState = PARSE_IN_LINE_COMMENT;
            c += 2;
        }
        else if (*c == '/' && *(c+1) == '*') {
            parseState = PARSE_IN_C_COMMENT;
            c += 2;
        }
        else if (t && IS_TOKEN_DELIMITER(*c)) {
            int tokenLen = c - t;
            switch (statementState) {
            case PRECISION:
                if (tokenLen == 9 && !strncmp(t,"precision",9)) {
                    statementState = QUALIFIER;
                    precision = t;
                }
                break;
            case QUALIFIER:
                if ((tokenLen == 4 && !strncmp(t,"lowp",4)) ||
                    (tokenLen == 7 && !strncmp(t,"mediump",7)) ||
                    (tokenLen == 5 && !strncmp(t,"highp",5))) {
                    statementState = SEMICOLON;
                }
                else {
                    statementState = PRECISION;
                }
                break;
            case SEMICOLON:
                if (*c == ';') {
                    for (char *r = (char *)precision; r<=c ; ++r) {
                        *r = ' '; //blank the character
                    }
                }
                statementState = PRECISION; //search for the next precision line
                break;
            default:
                break;
            }
            c++;
            t = NULL;
        }
        else if (IS_DELIMITER(*c)) {
            c++;
        }
        else {
            if (!t && IS_TOKEN_START(*c)) {
                t = c;
            }
            c++;
        }
    }
}

void ShaderParser::parseExtendDefaultPrecision(){

    //the precision lines which we need to add to the shader
    static const GLchar extend[] = {
                                      "#define GLES 1\n"
                                      "precision lowp sampler2D;\n"
                                      "precision lowp samplerCube;\n"
                                   };

    m_parsedSrc+=extend;
}

void ShaderParser::clearParsedSrc(){
    m_parsedSrc.clear();
}

GLenum ShaderParser::getType() {
    return m_type;
}

void ShaderParser::setInfoLog(GLchar* infoLog)
{
    delete[] m_infoLog;
    m_infoLog = infoLog;
}

GLchar* ShaderParser::getInfoLog()
{   
    return m_infoLog;
}

ShaderParser::~ShaderParser(){
    clearParsedSrc();
    if (m_originalSrc)
        free(m_originalSrc);
    delete[] m_infoLog;
}
