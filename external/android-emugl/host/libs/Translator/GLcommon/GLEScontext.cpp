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

#include <GLcommon/GLEScontext.h>
#include <GLcommon/GLconversion_macros.h>
#include <GLcommon/GLESmacros.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <OpenglCodecCommon/ErrorLog.h>
#include <GLcommon/GLESvalidate.h>
#include <GLcommon/TextureUtils.h>
#include <GLcommon/FramebufferData.h>
#include <strings.h>
#include <string.h>

//decleration
static void convertFixedDirectLoop(const char* dataIn,unsigned int strideIn,void* dataOut,unsigned int nBytes,unsigned int strideOut,int attribSize);
static void convertFixedIndirectLoop(const char* dataIn,unsigned int strideIn,void* dataOut,GLsizei count,GLenum indices_type,const GLvoid* indices,unsigned int strideOut,int attribSize);
static void convertByteDirectLoop(const char* dataIn,unsigned int strideIn,void* dataOut,unsigned int nBytes,unsigned int strideOut,int attribSize);
static void convertByteIndirectLoop(const char* dataIn,unsigned int strideIn,void* dataOut,GLsizei count,GLenum indices_type,const GLvoid* indices,unsigned int strideOut,int attribSize);

GLESConversionArrays::~GLESConversionArrays() {
    for(std::map<GLenum,ArrayData>::iterator it = m_arrays.begin(); it != m_arrays.end();it++) {
        if((*it).second.allocated){
            if((*it).second.type == GL_FLOAT){
                GLfloat* p = (GLfloat *)((*it).second.data);
                if(p) delete[] p;
            } else if((*it).second.type == GL_SHORT){
                GLshort* p = (GLshort *)((*it).second.data);
                if(p) delete[] p;
            }
        }
    }
}

void GLESConversionArrays::allocArr(unsigned int size,GLenum type){
    if(type == GL_FIXED){
        m_arrays[m_current].data = new GLfloat[size];
        m_arrays[m_current].type = GL_FLOAT;
    } else if(type == GL_BYTE){
        m_arrays[m_current].data = new GLshort[size];
        m_arrays[m_current].type = GL_SHORT;
    }
    m_arrays[m_current].stride = 0;
    m_arrays[m_current].allocated = true;
}

void GLESConversionArrays::setArr(void* data,unsigned int stride,GLenum type){
   m_arrays[m_current].type = type;
   m_arrays[m_current].data = data;
   m_arrays[m_current].stride = stride;
   m_arrays[m_current].allocated = false;
}

void* GLESConversionArrays::getCurrentData(){
    return m_arrays[m_current].data;
}

ArrayData& GLESConversionArrays::getCurrentArray(){
    return m_arrays[m_current];
}

unsigned int GLESConversionArrays::getCurrentIndex(){
    return m_current;
}

ArrayData& GLESConversionArrays::operator[](int i){
    return m_arrays[i];
}

void GLESConversionArrays::operator++(){
    m_current++;
}

GLDispatch     GLEScontext::s_glDispatch;
emugl::Mutex   GLEScontext::s_lock;
std::string*   GLEScontext::s_glExtensions= NULL;
std::string    GLEScontext::s_glVendor;
std::string    GLEScontext::s_glRenderer;
std::string    GLEScontext::s_glVersion;
GLSupport      GLEScontext::s_glSupport;

Version::Version():m_major(0),
                   m_minor(0),
                   m_release(0){};

Version::Version(int major,int minor,int release):m_major(major),
                                                  m_minor(minor),
                                                  m_release(release){};

Version::Version(const Version& ver):m_major(ver.m_major),
                                     m_minor(ver.m_minor),
                                     m_release(ver.m_release){}

Version::Version(const char* versionString){
    m_release = 0;
    if((!versionString) ||
      ((!(sscanf(versionString,"%d.%d"   ,&m_major,&m_minor) == 2)) &&
       (!(sscanf(versionString,"%d.%d.%d",&m_major,&m_minor,&m_release) == 3)))){
        m_major = m_minor = 0; // the version is not in the right format
    }
}

Version& Version::operator=(const Version& ver){
    m_major   = ver.m_major;
    m_minor   = ver.m_minor;
    m_release = ver.m_release;
    return *this;
}

bool Version::operator<(const Version& ver) const{
    if(m_major < ver.m_major) return true;
    if(m_major == ver.m_major){
        if(m_minor < ver.m_minor) return true;
        if(m_minor == ver.m_minor){
           return m_release < ver.m_release;
        }
    }
    return false;
}

static std::string getHostExtensionsString(GLDispatch* dispatch) {
    // glGetString(GL_EXTENSIONS) is deprecated in GL 3.0, one has to use
    // glGetStringi(GL_EXTENSIONS, index) instead to get individual extension
    // names. Recent desktop drivers implement glGetStringi() but have a
    // version of glGetString() that returns NULL, so deal with this by
    // doing the following:
    //
    //  - If glGetStringi() is available, and glGetIntegerv(GL_NUM_EXTENSIONS &num_exts)
    //    gives a nonzero value, use it to build the extensions
    //    string, using simple spaces to separate the names.
    //
    //  - Otherwise, fallback to getGetString(). If it returns NULL, return
    //    an empty string.
    //

    std::string result;
    int num_exts = 0;

    if (dispatch->glGetStringi) {
        dispatch->glGetIntegerv(GL_NUM_EXTENSIONS, &num_exts);
        GLenum err = dispatch->glGetError();
        if (err == GL_NO_ERROR) {
            for (int n = 0; n < num_exts; n++) {
                const char* ext = reinterpret_cast<const char*>(
                        dispatch->glGetStringi(GL_EXTENSIONS, n));
                if (ext != NULL) {
                    if (!result.empty()) {
                        result += " ";
                    }
                    result += ext;
                }
            }
        }
    }

    // If glGetIntegerv does not affect the value,
    // our system does not actually support
    // GL 3.0 style extension getting.
    if (!dispatch->glGetStringi || num_exts == 0) {
        const char* extensions = reinterpret_cast<const char*>(
                dispatch->glGetString(GL_EXTENSIONS));
        if (extensions) {
            result = extensions;
        }
    }

    // For the sake of initCapsLocked() add a starting and trailing space.
    if (!result.empty()) {
        if (result[0] != ' ') {
            result.insert(0, 1, ' ');
        }
        if (result[result.size() - 1U] != ' ') {
            result += ' ';
        }
    }
    return result;
}

static GLuint getIndex(GLenum indices_type, const GLvoid* indices, unsigned int i) {
    switch (indices_type) {
        case GL_UNSIGNED_BYTE:
            return static_cast<const GLubyte*>(indices)[i];
        case GL_UNSIGNED_SHORT:
            return static_cast<const GLushort*>(indices)[i];
        case GL_UNSIGNED_INT:
            return static_cast<const GLuint*>(indices)[i];
        default:
            ERR("**** ERROR unknown type 0x%x (%s,%d)\n", indices_type, __FUNCTION__,__LINE__);
            return 0;
    }
}

void GLEScontext::init(GlLibrary* glLib) {

    if (!s_glExtensions) {
        initCapsLocked(reinterpret_cast<const GLubyte*>(
                getHostExtensionsString(&s_glDispatch).c_str()));
        // NOTE: the string below corresponds to the extensions reported
        // by this context, which is initialized in each GLESv1 or GLESv2
        // context implementation, based on the parsing of the host
        // extensions string performed by initCapsLocked(). I.e. it will
        // be populated after calling this ::init() method.
        s_glExtensions = new std::string("");
    }

    if (!m_initialized) {
        initExtensionString();

        int maxTexUnits = getMaxTexUnits();
        m_texState = new textureUnitState[maxTexUnits];
        for (int i=0;i<maxTexUnits;++i) {
            for (int j=0;j<NUM_TEXTURE_TARGETS;++j)
            {
                m_texState[i][j].texture = 0;
                m_texState[i][j].enabled = GL_FALSE;
            }
        }
    }
}

GLEScontext::GLEScontext():
                           m_initialized(false)    ,
                           m_activeTexture(0)      ,
                           m_unpackAlignment(4)    ,
                           m_glError(GL_NO_ERROR)  ,
                           m_texState(0)          ,
                           m_arrayBuffer(0)        ,
                           m_elementBuffer(0),
                           m_renderbuffer(0),
                           m_framebuffer(0)
{
};

GLenum GLEScontext::getGLerror() {
    return m_glError;
}

void GLEScontext::setGLerror(GLenum err) {
    m_glError = err;
}

void GLEScontext::setActiveTexture(GLenum tex) {
   m_activeTexture = tex - GL_TEXTURE0;
}

GLEScontext::~GLEScontext() {
    for(ArraysMap::iterator it = m_map.begin(); it != m_map.end();it++) {
        GLESpointer* p = (*it).second;
        if(p) {
            delete p;
        }
    }
    delete[] m_texState;
    m_texState = NULL;
}

const GLvoid* GLEScontext::setPointer(GLenum arrType,GLint size,GLenum type,GLsizei stride,const GLvoid* data,bool normalize) {
    GLuint bufferName = m_arrayBuffer;
    if(bufferName) {
        unsigned int offset = SafeUIntFromPointer(data);
        GLESbuffer* vbo = static_cast<GLESbuffer*>(m_shareGroup->getObjectData(VERTEXBUFFER,bufferName).Ptr());
        m_map[arrType]->setBuffer(size,type,stride,vbo,bufferName,offset,normalize);
        return  static_cast<const unsigned char*>(vbo->getData()) +  offset;
    }
    m_map[arrType]->setArray(size,type,stride,data,normalize);
    return data;
}

void GLEScontext::enableArr(GLenum arr,bool enable) {
    m_map[arr]->enable(enable);
}

bool GLEScontext::isArrEnabled(GLenum arr) {
    return m_map[arr]->isEnable();
}

const GLESpointer* GLEScontext::getPointer(GLenum arrType) {
    if (m_map.find(arrType) != m_map.end()) return m_map[arrType];
    return NULL;
}

static void convertFixedDirectLoop(const char* dataIn,unsigned int strideIn,void* dataOut,unsigned int nBytes,unsigned int strideOut,int attribSize) {

    for(unsigned int i = 0; i < nBytes;i+=strideOut) {
        const GLfixed* fixed_data = (const GLfixed *)dataIn;
        //filling attrib
        for(int j=0;j<attribSize;j++) {
            reinterpret_cast<GLfloat*>(&static_cast<unsigned char*>(dataOut)[i])[j] = X2F(fixed_data[j]);
        }
        dataIn += strideIn;
    }
}

static void convertFixedIndirectLoop(const char* dataIn,unsigned int strideIn,void* dataOut,GLsizei count,GLenum indices_type,const GLvoid* indices,unsigned int strideOut,int attribSize) {
    for(int i = 0 ;i < count ;i++) {
        GLuint index = getIndex(indices_type, indices, i);

        const GLfixed* fixed_data = (GLfixed *)(dataIn  + index*strideIn);
        GLfloat* float_data = reinterpret_cast<GLfloat*>(static_cast<unsigned char*>(dataOut) + index*strideOut);

        for(int j=0;j<attribSize;j++) {
            float_data[j] = X2F(fixed_data[j]);
         }
    }
}

static void convertByteDirectLoop(const char* dataIn,unsigned int strideIn,void* dataOut,unsigned int nBytes,unsigned int strideOut,int attribSize) {

    for(unsigned int i = 0; i < nBytes;i+=strideOut) {
        const GLbyte* byte_data = (const GLbyte *)dataIn;
        //filling attrib
        for(int j=0;j<attribSize;j++) {
            reinterpret_cast<GLshort*>(&static_cast<unsigned char*>(dataOut)[i])[j] = B2S(byte_data[j]);
        }
        dataIn += strideIn;
    }
}

static void convertByteIndirectLoop(const char* dataIn,unsigned int strideIn,void* dataOut,GLsizei count,GLenum indices_type,const GLvoid* indices,unsigned int strideOut,int attribSize) {
    for(int i = 0 ;i < count ;i++) {
        GLuint index = getIndex(indices_type, indices, i);
        const GLbyte* bytes_data = (GLbyte *)(dataIn  + index*strideIn);
        GLshort* short_data = reinterpret_cast<GLshort*>(static_cast<unsigned char*>(dataOut) + index*strideOut);

        for(int j=0;j<attribSize;j++) {
            short_data[j] = B2S(bytes_data[j]);
         }
    }
}
static void directToBytesRanges(GLint first,GLsizei count,GLESpointer* p,RangeList& list) {

    int attribSize = p->getSize()*4; //4 is the sizeof GLfixed or GLfloat in bytes
    int stride = p->getStride()?p->getStride():attribSize;
    int start  = p->getBufferOffset()+first*attribSize;
    if(!p->getStride()) {
        list.addRange(Range(start,count*attribSize));
    } else {
        for(int i = 0 ;i < count; i++,start+=stride) {
            list.addRange(Range(start,attribSize));
        }
    }
}

static void indirectToBytesRanges(const GLvoid* indices,GLenum indices_type,GLsizei count,GLESpointer* p,RangeList& list) {

    int attribSize = p->getSize() * 4; //4 is the sizeof GLfixed or GLfloat in bytes
    int stride = p->getStride()?p->getStride():attribSize;
    int start  = p->getBufferOffset();
    for(int i=0 ; i < count; i++) {
        GLuint index = getIndex(indices_type, indices, i);
        list.addRange(Range(start+index*stride,attribSize));

    }
}

int bytesRangesToIndices(RangeList& ranges,GLESpointer* p,GLuint* indices) {

    int attribSize = p->getSize() * 4; //4 is the sizeof GLfixed or GLfloat in bytes
    int stride = p->getStride()?p->getStride():attribSize;
    int offset = p->getBufferOffset();

    int n = 0;
    for(int i=0;i<ranges.size();i++) {
        int startIndex = (ranges[i].getStart() - offset) / stride;
        int nElements = ranges[i].getSize()/attribSize;
        for(int j=0;j<nElements;j++) {
            indices[n++] = startIndex+j;
        }
    }
    return n;
}

void GLEScontext::convertDirect(GLESConversionArrays& cArrs,GLint first,GLsizei count,GLenum array_id,GLESpointer* p) {

    GLenum type    = p->getType();
    int attribSize = p->getSize();
    unsigned int size = attribSize*count + first;
    unsigned int bytes = type == GL_FIXED ? sizeof(GLfixed):sizeof(GLbyte);
    cArrs.allocArr(size,type);
    int stride = p->getStride()?p->getStride():bytes*attribSize;
    const char* data = (const char*)p->getArrayData() + (first*stride);

    if(type == GL_FIXED) {
        convertFixedDirectLoop(data,stride,cArrs.getCurrentData(),size*sizeof(GLfloat),attribSize*sizeof(GLfloat),attribSize);
    } else if(type == GL_BYTE) {
        convertByteDirectLoop(data,stride,cArrs.getCurrentData(),size*sizeof(GLshort),attribSize*sizeof(GLshort),attribSize);
    }
}

void GLEScontext::convertDirectVBO(GLESConversionArrays& cArrs,GLint first,GLsizei count,GLenum array_id,GLESpointer* p) {

    RangeList ranges;
    RangeList conversions;
    GLuint* indices = NULL;
    int attribSize = p->getSize();
    int stride = p->getStride()?p->getStride():sizeof(GLfixed)*attribSize;
    char* data = (char*)p->getBufferData() + (first*stride);

    if(p->bufferNeedConversion()) {
        directToBytesRanges(first,count,p,ranges); //converting indices range to buffer bytes ranges by offset
        p->getBufferConversions(ranges,conversions); // getting from the buffer the relevant ranges that still needs to be converted

        if(conversions.size()) { // there are some elements to convert
           indices = new GLuint[count];
           int nIndices = bytesRangesToIndices(conversions,p,indices); //converting bytes ranges by offset to indices in this array
           convertFixedIndirectLoop(data,stride,data,nIndices,GL_UNSIGNED_INT,indices,stride,attribSize);
        }
    }
    if(indices) delete[] indices;
    cArrs.setArr(data,p->getStride(),GL_FLOAT);
}

unsigned int GLEScontext::findMaxIndex(GLsizei count,GLenum type,const GLvoid* indices) {
    //finding max index
    unsigned int max = 0;
    if(type == GL_UNSIGNED_BYTE) {
        GLubyte*  b_indices  =(GLubyte *)indices;
        for(int i=0;i<count;i++) {
            if(b_indices[i] > max) max = b_indices[i];
        }
    } else if (type == GL_UNSIGNED_SHORT) {
        GLushort* us_indices =(GLushort *)indices;
        for(int i=0;i<count;i++) {
            if(us_indices[i] > max) max = us_indices[i];
        }
    } else { // type == GL_UNSIGNED_INT
        GLuint* ui_indices =(GLuint *)indices;
        for(int i=0;i<count;i++) {
            if(ui_indices[i] > max) max = ui_indices[i];
        }
    }
    return max;
}

void GLEScontext::convertIndirect(GLESConversionArrays& cArrs,GLsizei count,GLenum indices_type,const GLvoid* indices,GLenum array_id,GLESpointer* p) {
    GLenum type    = p->getType();
    int maxElements = findMaxIndex(count,indices_type,indices) + 1;

    int attribSize = p->getSize();
    int size = attribSize * maxElements;
    unsigned int bytes = type == GL_FIXED ? sizeof(GLfixed):sizeof(GLbyte);
    cArrs.allocArr(size,type);
    int stride = p->getStride()?p->getStride():bytes*attribSize;

    const char* data = (const char*)p->getArrayData();
    if(type == GL_FIXED) {
        convertFixedIndirectLoop(data,stride,cArrs.getCurrentData(),count,indices_type,indices,attribSize*sizeof(GLfloat),attribSize);
    } else if(type == GL_BYTE){
        convertByteIndirectLoop(data,stride,cArrs.getCurrentData(),count,indices_type,indices,attribSize*sizeof(GLshort),attribSize);
    }
}

void GLEScontext::convertIndirectVBO(GLESConversionArrays& cArrs,GLsizei count,GLenum indices_type,const GLvoid* indices,GLenum array_id,GLESpointer* p) {
    RangeList ranges;
    RangeList conversions;
    GLuint* conversionIndices = NULL;
    int attribSize = p->getSize();
    int stride = p->getStride()?p->getStride():sizeof(GLfixed)*attribSize;
    char* data = static_cast<char*>(p->getBufferData());
    if(p->bufferNeedConversion()) {
        indirectToBytesRanges(indices,indices_type,count,p,ranges); //converting indices range to buffer bytes ranges by offset
        p->getBufferConversions(ranges,conversions); // getting from the buffer the relevant ranges that still needs to be converted
        if(conversions.size()) { // there are some elements to convert
            conversionIndices = new GLuint[count];
            int nIndices = bytesRangesToIndices(conversions,p,conversionIndices); //converting bytes ranges by offset to indices in this array
            convertFixedIndirectLoop(data,stride,data,nIndices,GL_UNSIGNED_INT,conversionIndices,stride,attribSize);
        }
    }
    if(conversionIndices) delete[] conversionIndices;
    cArrs.setArr(data,p->getStride(),GL_FLOAT);
}



void GLEScontext::bindBuffer(GLenum target,GLuint buffer) {
    if(target == GL_ARRAY_BUFFER) {
        m_arrayBuffer = buffer;
    } else {
       m_elementBuffer = buffer;
    }
}

void GLEScontext::unbindBuffer(GLuint buffer) {
    if(m_arrayBuffer == buffer)
    {
        m_arrayBuffer = 0;
    }
    if(m_elementBuffer == buffer)
    {
        m_elementBuffer = 0;
    }
}

//checks if any buffer is binded to target
bool GLEScontext::isBindedBuffer(GLenum target) {
    if(target == GL_ARRAY_BUFFER) {
        return m_arrayBuffer != 0;
    } else {
        return m_elementBuffer != 0;
    }
}

GLuint GLEScontext::getBuffer(GLenum target) {
    return target == GL_ARRAY_BUFFER ? m_arrayBuffer:m_elementBuffer;
}

GLvoid* GLEScontext::getBindedBuffer(GLenum target) {
    GLuint bufferName = getBuffer(target);
    if(!bufferName) return NULL;

    GLESbuffer* vbo = static_cast<GLESbuffer*>(m_shareGroup->getObjectData(VERTEXBUFFER,bufferName).Ptr());
    return vbo->getData();
}

void GLEScontext::getBufferSize(GLenum target,GLint* param) {
    GLuint bufferName = getBuffer(target);
    GLESbuffer* vbo = static_cast<GLESbuffer*>(m_shareGroup->getObjectData(VERTEXBUFFER,bufferName).Ptr());
    *param = vbo->getSize();
}

void GLEScontext::getBufferUsage(GLenum target,GLint* param) {
    GLuint bufferName = getBuffer(target);
    GLESbuffer* vbo = static_cast<GLESbuffer*>(m_shareGroup->getObjectData(VERTEXBUFFER,bufferName).Ptr());
    *param = vbo->getUsage();
}

bool GLEScontext::setBufferData(GLenum target,GLsizeiptr size,const GLvoid* data,GLenum usage) {
    GLuint bufferName = getBuffer(target);
    if(!bufferName) return false;
    GLESbuffer* vbo = static_cast<GLESbuffer*>(m_shareGroup->getObjectData(VERTEXBUFFER,bufferName).Ptr());
    return vbo->setBuffer(size,usage,data);
}

bool GLEScontext::setBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,const GLvoid* data) {

    GLuint bufferName = getBuffer(target);
    if(!bufferName) return false;
    GLESbuffer* vbo = static_cast<GLESbuffer*>(m_shareGroup->getObjectData(VERTEXBUFFER,bufferName).Ptr());
    return vbo->setSubBuffer(offset,size,data);
}

const char * GLEScontext::getExtensionString() {
    const char * ret;
    s_lock.lock();
    if (s_glExtensions)
        ret = s_glExtensions->c_str();
    else
        ret="";
    s_lock.unlock();
    return ret;
}

const char * GLEScontext::getVendorString() const {
    return s_glVendor.c_str();
}

const char * GLEScontext::getRendererString() const {
    return s_glRenderer.c_str();
}

const char * GLEScontext::getVersionString() const {
    return s_glVersion.c_str();
}

void GLEScontext::getGlobalLock() {
    s_lock.lock();
}

void GLEScontext::releaseGlobalLock() {
    s_lock.unlock();
}


void GLEScontext::initCapsLocked(const GLubyte * extensionString)
{
    const char* cstring = (const char*)extensionString;

    s_glDispatch.glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,&s_glSupport.maxVertexAttribs);
    s_glDispatch.glGetIntegerv(GL_MAX_CLIP_PLANES,&s_glSupport.maxClipPlane);
    s_glDispatch.glGetIntegerv(GL_MAX_LIGHTS,&s_glSupport.maxLights);
    s_glDispatch.glGetIntegerv(GL_MAX_TEXTURE_SIZE,&s_glSupport.maxTexSize);
    s_glDispatch.glGetIntegerv(GL_MAX_TEXTURE_UNITS,&s_glSupport.maxTexUnits);
    s_glDispatch.glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&s_glSupport.maxTexImageUnits);
    s_glDispatch.glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &s_glSupport.maxCombinedTexImageUnits);
    const GLubyte* glslVersion = s_glDispatch.glGetString(GL_SHADING_LANGUAGE_VERSION);
    s_glSupport.glslVersion = Version((const  char*)(glslVersion));
    const GLubyte* glVersion = s_glDispatch.glGetString(GL_VERSION);

    if (strstr(cstring,"GL_EXT_bgra ")!=NULL)
        s_glSupport.GL_EXT_TEXTURE_FORMAT_BGRA8888 = true;

    if (strstr(cstring,"GL_EXT_framebuffer_object ")!=NULL)
        s_glSupport.GL_EXT_FRAMEBUFFER_OBJECT = true;

    if (strstr(cstring,"GL_ARB_vertex_blend ")!=NULL)
        s_glSupport.GL_ARB_VERTEX_BLEND = true;

    if (strstr(cstring,"GL_ARB_matrix_palette ")!=NULL)
        s_glSupport.GL_ARB_MATRIX_PALETTE = true;

    if (strstr(cstring,"GL_EXT_packed_depth_stencil ")!=NULL )
        s_glSupport.GL_EXT_PACKED_DEPTH_STENCIL = true;

    if (strstr(cstring,"GL_OES_read_format ")!=NULL)
        s_glSupport.GL_OES_READ_FORMAT = true;

    if (strstr(cstring,"GL_ARB_half_float_pixel ")!=NULL)
        s_glSupport.GL_ARB_HALF_FLOAT_PIXEL = true;

    if (strstr(cstring,"GL_NV_half_float ")!=NULL)
        s_glSupport.GL_NV_HALF_FLOAT = true;

    if (strstr(cstring,"GL_ARB_half_float_vertex ")!=NULL)
        s_glSupport.GL_ARB_HALF_FLOAT_VERTEX = true;

    if (strstr(cstring,"GL_SGIS_generate_mipmap ")!=NULL)
        s_glSupport.GL_SGIS_GENERATE_MIPMAP = true;

    if (strstr(cstring,"GL_ARB_ES2_compatibility ")!=NULL)
        s_glSupport.GL_ARB_ES2_COMPATIBILITY = true;

    if (strstr(cstring,"GL_OES_standard_derivatives ")!=NULL)
        s_glSupport.GL_OES_STANDARD_DERIVATIVES = true;

    if (strstr(cstring,"GL_ARB_texture_non_power_of_two")!=NULL)
        s_glSupport.GL_OES_TEXTURE_NPOT = true;

    if (!(Version((const char*)glVersion) < Version("3.0")) || strstr(cstring,"GL_OES_rgb8_rgba8")!=NULL)
        s_glSupport.GL_OES_RGB8_RGBA8 = true;

}

void GLEScontext::buildStrings(const char* baseVendor,
        const char* baseRenderer, const char* baseVersion, const char* version)
{
    static const char VENDOR[]   = {"Google ("};
    static const char RENDERER[] = {"Android Emulator OpenGL ES Translator ("};
    const size_t VENDOR_LEN   = sizeof(VENDOR) - 1;
    const size_t RENDERER_LEN = sizeof(RENDERER) - 1;

    // Sanitize the strings as some OpenGL implementations return NULL
    // when asked the basic questions (this happened at least once on a client
    // machine)
    if (!baseVendor) {
        baseVendor = "N/A";
    }
    if (!baseRenderer) {
        baseRenderer = "N/A";
    }
    if (!baseVersion) {
        baseVersion = "N/A";
    }
    if (!version) {
        version = "N/A";
    }

    size_t baseVendorLen = strlen(baseVendor);
    s_glVendor.clear();
    s_glVendor.reserve(baseVendorLen + VENDOR_LEN + 1);
    s_glVendor.append(VENDOR, VENDOR_LEN);
    s_glVendor.append(baseVendor, baseVendorLen);
    s_glVendor.append(")", 1);

    size_t baseRendererLen = strlen(baseRenderer);
    s_glRenderer.clear();
    s_glRenderer.reserve(baseRendererLen + RENDERER_LEN + 1);
    s_glRenderer.append(RENDERER, RENDERER_LEN);
    s_glRenderer.append(baseRenderer, baseRendererLen);
    s_glRenderer.append(")", 1);

    size_t baseVersionLen = strlen(baseVersion);
    size_t versionLen = strlen(version);
    s_glVersion.clear();
    s_glVersion.reserve(baseVersionLen + versionLen + 3);
    s_glVersion.append(version, versionLen);
    s_glVersion.append(" (", 2);
    s_glVersion.append(baseVersion, baseVersionLen);
    s_glVersion.append(")", 1);
}

bool GLEScontext::isTextureUnitEnabled(GLenum unit) {
    for (int i=0;i<NUM_TEXTURE_TARGETS;++i) {
        if (m_texState[unit-GL_TEXTURE0][i].enabled)
            return true;
    }
    return false;
}

bool GLEScontext::glGetBooleanv(GLenum pname, GLboolean *params)
{
    GLint iParam;

    if(glGetIntegerv(pname, &iParam))
    {
        *params = (iParam != 0);
        return true;
    }

    return false;
}

bool GLEScontext::glGetFixedv(GLenum pname, GLfixed *params)
{
    bool result = false;
    GLint numParams = 1;

    GLint* iParams = new GLint[numParams];
    if (numParams>0 && glGetIntegerv(pname,iParams)) {
        while(numParams >= 0)
        {
            params[numParams] = I2X(iParams[numParams]);
            numParams--;
        }
        result = true;
    }
    delete [] iParams;

    return result;
}

bool GLEScontext::glGetFloatv(GLenum pname, GLfloat *params)
{
    bool result = false;
    GLint numParams = 1;

    GLint* iParams = new GLint[numParams];
    if (numParams>0 && glGetIntegerv(pname,iParams)) {
        while(numParams >= 0)
        {
            params[numParams] = (GLfloat)iParams[numParams];
            numParams--;
        }
        result = true;
    }
    delete [] iParams;

    return result;
}        

bool GLEScontext::glGetIntegerv(GLenum pname, GLint *params)
{
    switch(pname)
    {
        case GL_ARRAY_BUFFER_BINDING:
            *params = m_arrayBuffer;
            break;

        case GL_ELEMENT_ARRAY_BUFFER_BINDING:
            *params = m_elementBuffer;
            break;

        case GL_TEXTURE_BINDING_CUBE_MAP:
            *params = m_texState[m_activeTexture][TEXTURE_CUBE_MAP].texture;
            break;

        case GL_TEXTURE_BINDING_2D:
            *params = m_texState[m_activeTexture][TEXTURE_2D].texture;
            break;

        case GL_ACTIVE_TEXTURE:
            *params = m_activeTexture+GL_TEXTURE0;
            break;

        case GL_IMPLEMENTATION_COLOR_READ_TYPE_OES:
            *params = GL_UNSIGNED_BYTE;
            break;

        case GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES:
            *params = GL_RGBA;
            break;

        case GL_MAX_TEXTURE_SIZE:
            *params = getMaxTexSize();
            break;
        default:
            return false;
    }

    return true;
}

TextureTarget GLEScontext::GLTextureTargetToLocal(GLenum target) {
    TextureTarget value=TEXTURE_2D;
    switch (target) {
    case GL_TEXTURE_CUBE_MAP:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        value = TEXTURE_CUBE_MAP;
        break;
    case GL_TEXTURE_2D:
        value = TEXTURE_2D;
        break;
    }
    return value;
}

unsigned int GLEScontext::getBindedTexture(GLenum target) {
    TextureTarget pos = GLTextureTargetToLocal(target);
    return m_texState[m_activeTexture][pos].texture;
}
  
unsigned int GLEScontext::getBindedTexture(GLenum unit, GLenum target) {
    TextureTarget pos = GLTextureTargetToLocal(target);
    return m_texState[unit-GL_TEXTURE0][pos].texture;
}

void GLEScontext::setBindedTexture(GLenum target, unsigned int tex) {
    TextureTarget pos = GLTextureTargetToLocal(target);
    m_texState[m_activeTexture][pos].texture = tex;
}

void GLEScontext::setTextureEnabled(GLenum target, GLenum enable) {
    TextureTarget pos = GLTextureTargetToLocal(target);
    m_texState[m_activeTexture][pos].enabled = enable;
}

#define INTERNAL_NAME(x) (x +0x100000000ll);

ObjectLocalName GLEScontext::getDefaultTextureName(GLenum target) {
    ObjectLocalName name = 0;
    switch (GLTextureTargetToLocal(target)) {
    case TEXTURE_2D:
        name = INTERNAL_NAME(0);
        break;
    case TEXTURE_CUBE_MAP:
        name = INTERNAL_NAME(1);
        break;
    default:
        name = 0;
        break;
    }
    return name;
}

void GLEScontext::drawValidate(void)
{
    if(m_framebuffer == 0)
        return;

    ObjectDataPtr fbObj = m_shareGroup->getObjectData(FRAMEBUFFER,m_framebuffer);
    if (fbObj.Ptr() == NULL)
        return;

    FramebufferData *fbData = (FramebufferData *)fbObj.Ptr();

    fbData->validate(this);
}
