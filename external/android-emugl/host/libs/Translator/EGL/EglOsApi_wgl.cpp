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
#include "EglOsApi.h"

#include "emugl/common/lazy_instance.h"
#include "emugl/common/shared_library.h"
#include "GLcommon/GLLibrary.h"

#include "OpenglCodecCommon/ErrorLog.h"

#include <windows.h>
#include <wingdi.h>

#include <GLES/glplatform.h>
#include <GL/gl.h>
#include <GL/wglext.h>

#include <map>

#include <stdio.h>
#include <stdlib.h>

#define IS_TRUE(a) \
        do { if (!(a)) return NULL; } while (0)

#define EXIT_IF_FALSE(a) \
        do { if (!(a)) return; } while (0)

#define DEBUG 0
#if DEBUG
#define D(...)  fprintf(stderr, __VA_ARGS__)
#else
#define D(...)  ((void)0)
#endif

namespace {

using emugl::SharedLibrary;
typedef GlLibrary::GlFunctionPointer GlFunctionPointer;

// Returns true if an extension is include in a given extension list.
// |extension| is an GL extension name.
// |extensionList| is a space-separated list of supported extension.
// Returns true if the extension is supported, false otherwise.
bool supportsExtension(const char* extension, const char* extensionList) {
    size_t extensionLen = ::strlen(extension);
    const char* list = extensionList;
    for (;;) {
        const char* p = const_cast<const char*>(::strstr(list, extension));
        if (!p) {
            return false;
        }
        // Check that the extension appears as a single word in the list
        // i.e. that it is wrapped by either spaces or the start/end of
        // the list.
        if ((p == extensionList || p[-1] == ' ') &&
            (p[extensionLen] == '\0' || p[extensionLen] == ' ')) {
            return true;
        }
        // otherwise, skip over the current position to find something else.
        p += extensionLen;
    }
}

/////
/////  W G L   D I S P A T C H   T A B L E S
/////

// A technical note to explain what is going here, trust me, it's important.
//
// I. Library-dependent symbol resolution:
// ---------------------------------------
//
// The code here can deal with two kinds of OpenGL Windows libraries: the
// system-provided opengl32.dll, or alternate software renderers like Mesa
// (e.g. mesa_opengl32.dll).
//
// When using the system library, pixel-format related functions, like
// SetPixelFormat(), are provided by gdi32.dll, and _not_ opengl32.dll
// (even though they are documented as part of the Windows GL API).
//
// These functions must _not_ be used when using alternative renderers.
// Instead, these provide _undocumented_ alternatives with the 'wgl' prefix,
// as in wglSetPixelFormat(), wglDescribePixelFormat(), etc... which
// implement the same calling conventions.
//
// For more details about this, see 5.190 of the Windows OpenGL FAQ at:
// https://www.opengl.org/archives/resources/faq/technical/mswindows.htm
//
// Another good source of information on this topic:
// http://stackoverflow.com/questions/20645706/why-are-functions-duplicated-between-opengl32-dll-and-gdi32-dll
//
// In practice, it means that the code here should resolve 'naked' symbols
// (e.g. 'GetPixelFormat()) when using the system library, and 'prefixed' ones
// (e.g. 'wglGetPixelFormat()) otherwise.
//

// List of WGL functions of interest to probe with GetProcAddress()
#define LIST_WGL_FUNCTIONS(X) \
    X(HGLRC, wglCreateContext, (HDC hdc)) \
    X(BOOL, wglDeleteContext, (HGLRC hglrc)) \
    X(BOOL, wglMakeCurrent, (HDC hdc, HGLRC hglrc)) \
    X(BOOL, wglShareLists, (HGLRC hglrc1, HGLRC hglrc2)) \
    X(HGLRC, wglGetCurrentContext, (void)) \
    X(HDC, wglGetCurrentDC, (void)) \
    X(GlFunctionPointer, wglGetProcAddress, (const char* functionName)) \

// List of WGL functions exported by GDI32 that must be used when using
// the system's opengl32.dll only, i.e. not with a software renderer like
// Mesa. For more information, see 5.190 at:
// And also:
#define LIST_GDI32_FUNCTIONS(X) \
    X(int, ChoosePixelFormat, (HDC hdc, const PIXELFORMATDESCRIPTOR* ppfd)) \
    X(BOOL, SetPixelFormat, (HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR* pfd)) \
    X(int, GetPixelFormat, (HDC hdc)) \
    X(int, DescribePixelFormat, (HDC hdc, int iPixelFormat, UINT nbytes, LPPIXELFORMATDESCRIPTOR ppfd)) \
    X(BOOL, SwapBuffers, (HDC hdc)) \

// Declare a structure containing pointers to all functions listed above,
// and a way to initialize them.
struct WglBaseDispatch {
    // declare all function pointers, followed by a dummy member used
    // to terminate the constructor's initialization list properly.
#define DECLARE_WGL_POINTER(return_type, function_name, signature) \
    return_type (GL_APIENTRY* function_name) signature;
    LIST_WGL_FUNCTIONS(DECLARE_WGL_POINTER)
    LIST_GDI32_FUNCTIONS(DECLARE_WGL_POINTER)
    SharedLibrary* mLib;
    bool mIsSystemLib;

    // Default Constructor
    WglBaseDispatch() :
#define INIT_WGL_POINTER(return_type, function_name, signature) \
    function_name(NULL),
            LIST_WGL_FUNCTIONS(INIT_WGL_POINTER)
            LIST_GDI32_FUNCTIONS(INIT_WGL_POINTER)
            mLib(NULL),
            mIsSystemLib(false) {}

    // Copy constructor
    WglBaseDispatch(const WglBaseDispatch& other) :
#define COPY_WGL_POINTER(return_type, function_name, signature) \
    function_name(other.function_name),
            LIST_WGL_FUNCTIONS(COPY_WGL_POINTER)
            LIST_GDI32_FUNCTIONS(COPY_WGL_POINTER)
            mLib(other.mLib),
            mIsSystemLib(other.mIsSystemLib) {}

    // Initialize the dispatch table from shared library |glLib|, which
    // must point to either the system or non-system opengl32.dll
    // implementation. If |systemLib| is true, this considers the library
    // to be the system one, and will try to find the GDI32 functions
    // like GetPixelFormat() directly. If |systemLib| is false, this will
    // try to load the wglXXX variants (e.g. for Mesa). See technical note
    // above for details.
    // Returns true on success, false otherwise (i.e. if one of the
    // required symbols could not be loaded).
    bool init(SharedLibrary* glLib, bool systemLib) {
        bool result = true;

        mLib = glLib;
        mIsSystemLib = systemLib;

#define LOAD_WGL_POINTER(return_type, function_name, signature) \
    this->function_name = reinterpret_cast< \
            return_type (GL_APIENTRY*) signature>( \
                    glLib->findSymbol(#function_name)); \
    if (!this->function_name) { \
        ERR("%s: Could not find %s in GL library\n", __FUNCTION__, \
                #function_name); \
        result = false; \
    }

#define LOAD_WGL_GDI32_POINTER(return_type, function_name, signature) \
    this->function_name = reinterpret_cast< \
            return_type (GL_APIENTRY*) signature>( \
                    GetProcAddress(gdi32, #function_name)); \
    if (!this->function_name) { \
        ERR("%s: Could not find %s in GDI32 library\n", __FUNCTION__, \
                #function_name); \
        result = false; \
    }

#define LOAD_WGL_INNER_POINTER(return_type, function_name, signature) \
    this->function_name = reinterpret_cast< \
            return_type (GL_APIENTRY*) signature>( \
                    glLib->findSymbol("wgl" #function_name)); \
    if (!this->function_name) { \
        ERR("%s: Could not find %s in GL library\n", __FUNCTION__, \
                "wgl" #function_name); \
        result = false; \
    }

        LIST_WGL_FUNCTIONS(LOAD_WGL_POINTER)
        if (systemLib) {
            HMODULE gdi32 = GetModuleHandle("gdi32.dll");
            LIST_GDI32_FUNCTIONS(LOAD_WGL_GDI32_POINTER)
        } else {
            LIST_GDI32_FUNCTIONS(LOAD_WGL_INNER_POINTER)
        }
        return result;
    }

    // Find a function, using wglGetProcAddress() first, and if that doesn't
    // work, SharedLibrary::findSymbol().
    // |functionName| is the function name.
    // |glLib| is the GL library to probe with findSymbol() is needed.
    GlFunctionPointer findFunction(const char* functionName) const {
        GlFunctionPointer result = this->wglGetProcAddress(functionName);
        if (!result && mLib) {
            result = mLib->findSymbol(functionName);
        }
        return result;
    }
};

// Used internally by createDummyWindow().
LRESULT CALLBACK dummyWndProc(HWND hwnd,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam) {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Create a new dummy window, and return its handle.
// Note that the window is 1x1 pixels and not visible, but
// it can be used to create a device context and associated
// OpenGL rendering context.
HWND createDummyWindow() {
    WNDCLASSEX wcx;
    wcx.cbSize = sizeof(wcx);                       // size of structure
    wcx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW; // redraw if size changes
    wcx.lpfnWndProc = dummyWndProc;                 // points to window procedure
    wcx.cbClsExtra = 0;                             // no extra class memory
    wcx.cbWndExtra = sizeof(void*);                 // save extra window memory, to store VasWindow instance
    wcx.hInstance = NULL;                           // handle to instance
    wcx.hIcon = NULL;                               // predefined app. icon
    wcx.hCursor = NULL;
    wcx.hbrBackground = NULL;                       // no background brush
    wcx.lpszMenuName =  NULL;                       // name of menu resource
    wcx.lpszClassName = "DummyWin";                 // name of window class
    wcx.hIconSm = (HICON) NULL;                     // small class icon

    RegisterClassEx(&wcx);

    HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
                               "DummyWin",
                               "Dummy",
                               WS_POPUP,
                               0,
                               0,
                               1,
                               1,
                               NULL,
                               NULL,
                               0,0);
    return hwnd;
}

// List of functions defined by the WGL_ARB_extensions_string extension.
#define LIST_WGL_ARB_extensions_string_FUNCTIONS(X) \
    X(const char*, wglGetExtensionsStringARB, (HDC hdc))

// List of functions defined by the WGL_ARB_pixel_format extension.
#define LIST_WGL_ARB_pixel_format_FUNCTIONS(X) \
    X(BOOL, wglGetPixelFormatAttribivARB, (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, int* piValues)) \
    X(BOOL, wglGetPixelFormatAttribfvARB, (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int* piAttributes, FLOAT* pfValues)) \
    X(BOOL, wglChoosePixelFormatARB, (HDC, const int* piAttribList, const FLOAT* pfAttribList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats)) \

// List of functions defined by the WGL_ARB_make_current_read extension.
#define LIST_WGL_ARB_make_current_read_FUNCTIONS(X) \
    X(BOOL, wglMakeContextCurrentARB, (HDC hDrawDC, HDC hReadDC, HGLRC hglrc)) \
    X(HDC, wglGetCurrentReadDCARB, (void)) \

// List of functions defined by the WGL_ARB_pbuffer extension.
#define LIST_WGL_ARB_pbuffer_FUNCTIONS(X) \
    X(HPBUFFERARB, wglCreatePbufferARB, (HDC hdc, int iPixelFormat, int iWidth, int iHeight, const int* piAttribList)) \
    X(HDC, wglGetPbufferDCARB, (HPBUFFERARB hPbuffer)) \
    X(int, wglReleasePbufferDCARB, (HPBUFFERARB hPbuffer, HDC hdc)) \
    X(BOOL, wglDestroyPbufferARB, (HPBUFFERARB hPbuffer)) \

#define LIST_WGL_EXTENSIONS_FUNCTIONS(X) \
    LIST_WGL_ARB_pixel_format_FUNCTIONS(X) \
    LIST_WGL_ARB_make_current_read_FUNCTIONS(X) \
    LIST_WGL_ARB_pbuffer_FUNCTIONS(X) \

// A structure used to hold pointers to WGL extension functions.
struct WglExtensionsDispatch : public WglBaseDispatch {
public:
    LIST_WGL_EXTENSIONS_FUNCTIONS(DECLARE_WGL_POINTER)
    int dummy;

    // Default constructor
    explicit WglExtensionsDispatch(const WglBaseDispatch& baseDispatch) :
            WglBaseDispatch(baseDispatch),
            LIST_WGL_EXTENSIONS_FUNCTIONS(INIT_WGL_POINTER)
            dummy(0) {}

    // Initialization
    bool init(HDC hdc) {
        // Base initialization happens first.
        bool result = WglBaseDispatch::init(mLib, mIsSystemLib);
        if (!result) {
            return false;
        }

        // Find the list of extensions.
        typedef const char* (GL_APIENTRY* GetExtensionsStringFunc)(HDC hdc);

        GetExtensionsStringFunc wglGetExtensionsStringARB =
                reinterpret_cast<GetExtensionsStringFunc>(
                        this->findFunction("wglGetExtensionsStringARB"));
        if (!wglGetExtensionsStringARB) {
            ERR("%s: Could not find wglGetExtensionsStringARB!\n",
                __FUNCTION__);
            return false;
        }
        const char* extensionList = wglGetExtensionsStringARB(hdc);
        if (!extensionList) {
            extensionList = "";
        }

        // Load each extension individually.
#define LOAD_WGL_EXTENSION_FUNCTION(return_type, function_name, signature) \
    this->function_name = reinterpret_cast< \
            return_type (GL_APIENTRY*) signature>( \
                    this->findFunction(#function_name)); \
    if (!this->function_name) { \
        ERR("ERROR: %s: Missing extension function %s\n", __FUNCTION__, \
            #function_name); \
        result = false; \
    }

#define LOAD_WGL_EXTENSION(extension) \
    if (!supportsExtension(#extension, extensionList)) { \
        ERR("WARNING: %s: Missing WGL extension %s\n", __FUNCTION__, #extension); \
    } else { \
        LIST_##extension##_FUNCTIONS(LOAD_WGL_EXTENSION_FUNCTION) \
    }

        LOAD_WGL_EXTENSION(WGL_ARB_pixel_format)
        LOAD_WGL_EXTENSION(WGL_ARB_make_current_read)
        LOAD_WGL_EXTENSION(WGL_ARB_pbuffer)

        // Done.
        return result;
    }

private:
    WglExtensionsDispatch();  // no default constructor.
};

const WglExtensionsDispatch* initExtensionsDispatch(
        const WglBaseDispatch* dispatch) {
    HWND hwnd = createDummyWindow();
    HDC hdc =  GetDC(hwnd);
    if (!hwnd || !hdc){
        fprintf(stderr,"error while getting DC\n");
        return NULL;
    }
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
        1,                     // version number
        PFD_DRAW_TO_WINDOW |   // support window
        PFD_SUPPORT_OPENGL |   // support OpenGL
        PFD_DOUBLEBUFFER,      // double buffered
        PFD_TYPE_RGBA,         // RGBA type
        32,                    // 32-bit color depth
        0, 0, 0, 0, 0, 0,      // color bits ignored
        0,                     // no alpha buffer
        0,                     // shift bit ignored
        0,                     // no accumulation buffer
        0, 0, 0, 0,            // accum bits ignored
        24,                    // 24-bit z-buffer
        0,                     // no stencil buffer
        0,                     // no auxiliary buffer
        PFD_MAIN_PLANE,        // main layer
        0,                     // reserved
        0, 0, 0                // layer masks ignored
    };

    int iPixelFormat = dispatch->ChoosePixelFormat(hdc, &pfd);
    if (iPixelFormat <= 0) {
        int err = GetLastError();
        fprintf(stderr,"error while choosing pixel format 0x%x\n", err);
        return NULL;
    }
    if (!dispatch->SetPixelFormat(hdc, iPixelFormat, &pfd)) {
        int err = GetLastError();
        fprintf(stderr,"error while setting pixel format 0x%x\n", err);
        return NULL;
    }

    int err;
    HGLRC ctx = dispatch->wglCreateContext(hdc);
    if (!ctx) {
        err =  GetLastError();
        fprintf(stderr,"error while creating dummy context %d\n", err);
    }
    if (!dispatch->wglMakeCurrent(hdc, ctx)) {
        err =  GetLastError();
        fprintf(stderr,"error while making dummy context current %d\n", err);
    }

    WglExtensionsDispatch* result = new WglExtensionsDispatch(*dispatch);
    result->init(hdc);

    dispatch->wglMakeCurrent(hdc, NULL);
    dispatch->wglDeleteContext(ctx);
    DeleteDC(hdc);
    DestroyWindow(hwnd);

    return result;
}

class WinPixelFormat : public EglOS::PixelFormat {
public:
    WinPixelFormat(const PIXELFORMATDESCRIPTOR* desc, int configId) {
        mDesc = *desc;
        mConfigId = configId;
    }

    EglOS::PixelFormat* clone() {
        return new WinPixelFormat(&mDesc, mConfigId);
    }

    const PIXELFORMATDESCRIPTOR* desc() const { return &mDesc; }
    const int configId() const { return mConfigId; }

    static const WinPixelFormat* from(const EglOS::PixelFormat* f) {
        return static_cast<const WinPixelFormat*>(f);
    }

private:
    WinPixelFormat();
    WinPixelFormat(const WinPixelFormat& other);

    PIXELFORMATDESCRIPTOR mDesc;
    int mConfigId;
};

class WinSurface : public EglOS::Surface {
public:
    explicit WinSurface(HWND wnd) :
            Surface(WINDOW),
            m_hwnd(wnd),
            m_pb(NULL),
            m_hdc(GetDC(wnd)) {}

    explicit WinSurface(HPBUFFERARB pb, const WglExtensionsDispatch* dispatch) :
            Surface(PBUFFER),
            m_hwnd(NULL),
            m_pb(pb),
            m_hdc(NULL) {
        if (dispatch->wglGetPbufferDCARB) {
            m_hdc = dispatch->wglGetPbufferDCARB(pb);
        }
    }

    ~WinSurface() {
        if (type() == WINDOW) {
            ReleaseDC(m_hwnd, m_hdc);
        }
    }

    HWND getHwnd() const { return m_hwnd; }
    HDC  getDC() const { return m_hdc; }
    HPBUFFERARB getPbuffer() const { return m_pb; }

    static WinSurface* from(EglOS::Surface* s) {
        return static_cast<WinSurface*>(s);
    }

private:
    HWND        m_hwnd;
    HPBUFFERARB m_pb;
    HDC         m_hdc;
};

class WinContext : public EglOS::Context {
public:
    explicit WinContext(HGLRC ctx) : mCtx(ctx) {}

    HGLRC context() const { return mCtx; }

    static HGLRC from(const EglOS::Context* c) {
        return static_cast<const WinContext*>(c)->context();
    }

private:
    HGLRC mCtx;
};

struct DisplayInfo {
    DisplayInfo() : dc(NULL), hwnd(NULL), isPixelFormatSet(false) {}

    DisplayInfo(HDC hdc, HWND wnd) :
            dc(hdc), hwnd(wnd), isPixelFormatSet(false) {}

    void release() {
        if (hwnd) {
            DestroyWindow(hwnd);
        }
        DeleteDC(dc);
    }

    HDC  dc;
    HWND hwnd;
    bool isPixelFormatSet;
};

struct TlsData {
    std::map<int, DisplayInfo> m_map;
};

DWORD s_tlsIndex = 0;

TlsData* getTLS() {
    TlsData *tls = (TlsData *)TlsGetValue(s_tlsIndex);
    if (!tls) {
        tls = new TlsData();
        TlsSetValue(s_tlsIndex, tls);
    }
    return tls;
}

}  // namespace

class WinDisplay {
public:
     enum { DEFAULT_DISPLAY = 0 };

     WinDisplay() {};

     DisplayInfo& getInfo(int configurationIndex) const {
         return getTLS()->m_map[configurationIndex];
     }

     HDC getDC(int configId) const {
         return getInfo(configId).dc;
     }

     void setInfo(int configurationIndex, const DisplayInfo& info);

     bool isPixelFormatSet(int cfgId) const {
         return getInfo(cfgId).isPixelFormatSet;
     }

     void pixelFormatWasSet(int cfgId) {
         getTLS()->m_map[cfgId].isPixelFormatSet = true;
     }

     bool infoExists(int configurationIndex);

     void releaseAll();
};

void WinDisplay::releaseAll(){
    TlsData * tls = getTLS();

    for(std::map<int,DisplayInfo>::iterator it = tls->m_map.begin();
        it != tls->m_map.end();
        it++) {
       (*it).second.release();
    }
}

bool WinDisplay::infoExists(int configurationIndex) {
    return getTLS()->m_map.find(configurationIndex) != getTLS()->m_map.end();
}

void WinDisplay::setInfo(int configurationIndex, const DisplayInfo& info) {
    getTLS()->m_map[configurationIndex] = info;
}

namespace {

static HDC getDummyDC(WinDisplay* display, int cfgId) {
    HDC dpy = NULL;
    if (!display->infoExists(cfgId)) {
        HWND hwnd = createDummyWindow();
        dpy  = GetDC(hwnd);
        display->setInfo(cfgId, DisplayInfo(dpy, hwnd));
    } else {
        dpy = display->getDC(cfgId);
    }
    return dpy;
}


bool initPixelFormat(HDC dc, const WglExtensionsDispatch* dispatch) {
    if (dispatch->wglChoosePixelFormatARB) {
        unsigned int numpf;
        int iPixelFormat;
        int i0 = 0;
        float f0 = 0.0f;
        return dispatch->wglChoosePixelFormatARB(
                dc, &i0, &f0, 1, &iPixelFormat, &numpf);
    } else {
        PIXELFORMATDESCRIPTOR  pfd;
        return dispatch->ChoosePixelFormat(dc, &pfd);
    }
}

void pixelFormatToConfig(WinDisplay* display,
                         const WglExtensionsDispatch* dispatch,
                         int renderableType,
                         const PIXELFORMATDESCRIPTOR* frmt,
                         int index,
                         EglOS::AddConfigCallback* addConfigFunc,
                         void* addConfigOpaque) {
    EglOS::ConfigInfo info;
    memset(&info, 0, sizeof(info));

    HDC dpy = getDummyDC(display, WinDisplay::DEFAULT_DISPLAY);

    if (frmt->iPixelType != PFD_TYPE_RGBA) {
        D("%s: Not an RGBA type!\n", __FUNCTION__);
        return; // other formats are not supported yet
    }
    if (!(frmt->dwFlags & PFD_SUPPORT_OPENGL)) {
        D("%s: No OpenGL support\n", __FUNCTION__);
        return;
    }
    // NOTE: Software renderers don't always support double-buffering.
    if (dispatch->mIsSystemLib && !(frmt->dwFlags & PFD_DOUBLEBUFFER)) {
        D("%s: No double-buffer support\n", __FUNCTION__);
        return;
    }
    if ((frmt->dwFlags & (PFD_GENERIC_FORMAT | PFD_NEED_PALETTE)) != 0) {
        //discard generic pixel formats as well as pallete pixel formats
        D("%s: Generic format or needs palette\n", __FUNCTION__);
        return;
    }

    if (!dispatch->wglGetPixelFormatAttribivARB) {
        D("%s: Missing wglGetPixelFormatAttribivARB\n", __FUNCTION__);
        return;
    }

    GLint window = 0, pbuffer = 0;
    if (dispatch->mIsSystemLib) {
        int windowAttrib = WGL_DRAW_TO_WINDOW_ARB;
        EXIT_IF_FALSE(dispatch->wglGetPixelFormatAttribivARB(
                dpy, index, 0, 1, &windowAttrib, &window));
    }
    int pbufferAttrib = WGL_DRAW_TO_PBUFFER_ARB;
    EXIT_IF_FALSE(dispatch->wglGetPixelFormatAttribivARB(
            dpy, index, 0, 1, &pbufferAttrib, &pbuffer));

    info.surface_type = 0;
    if (window) {
        info.surface_type |= EGL_WINDOW_BIT;
    }
    if (pbuffer) {
        info.surface_type |= EGL_PBUFFER_BIT;
    }
    if (!info.surface_type) {
        D("%s: Missing surface type\n", __FUNCTION__);
        return;
    }

    //default values
    info.native_visual_id = 0;
    info.native_visual_type = EGL_NONE;
    info.caveat = EGL_NONE;
    info.native_renderable = EGL_FALSE;
    info.renderable_type = renderableType;
    info.max_pbuffer_width = PBUFFER_MAX_WIDTH;
    info.max_pbuffer_height = PBUFFER_MAX_HEIGHT;
    info.max_pbuffer_size = PBUFFER_MAX_PIXELS;
    info.samples_per_pixel = 0;
    info.frame_buffer_level = 0;

    GLint transparent;
    int transparentAttrib = WGL_TRANSPARENT_ARB;
    EXIT_IF_FALSE(dispatch->wglGetPixelFormatAttribivARB(
            dpy, index, 0, 1, &transparentAttrib, &transparent));
    if (transparent) {
        info.transparent_type = EGL_TRANSPARENT_RGB;
        int transparentRedAttrib = WGL_TRANSPARENT_RED_VALUE_ARB;
        EXIT_IF_FALSE(dispatch->wglGetPixelFormatAttribivARB(
                dpy, index, 0, 1, &transparentRedAttrib, &info.trans_red_val));
        int transparentGreenAttrib = WGL_TRANSPARENT_GREEN_VALUE_ARB;
        EXIT_IF_FALSE(dispatch->wglGetPixelFormatAttribivARB(
                dpy, index, 0, 1, &transparentGreenAttrib,
                &info.trans_green_val));
        int transparentBlueAttrib = WGL_TRANSPARENT_RED_VALUE_ARB;
        EXIT_IF_FALSE(dispatch->wglGetPixelFormatAttribivARB(
                dpy,index, 0, 1, &transparentBlueAttrib, &info.trans_blue_val));
    } else {
        info.transparent_type = EGL_NONE;
    }

    info.red_size = frmt->cRedBits;
    info.green_size = frmt->cGreenBits;
    info.blue_size = frmt->cBlueBits;
    info.alpha_size = frmt->cAlphaBits;
    info.depth_size = frmt->cDepthBits;
    info.stencil_size = frmt->cStencilBits;

    info.config_id = (EGLint) index;
    info.frmt = new WinPixelFormat(frmt, index);

    (*addConfigFunc)(addConfigOpaque, &info);
}

class WglDisplay : public EglOS::Display {
public:
    // TODO(digit): Remove WinDisplay entirely.
    explicit WglDisplay(WinDisplay* dpy, const WglExtensionsDispatch* dispatch) :
            mDpy(dpy), mDispatch(dispatch) {}

    virtual ~WglDisplay() {
        delete mDpy;
    }

    virtual bool release() {
        mDpy->releaseAll();
        return true;
    }

    virtual void queryConfigs(int renderableType,
                              EglOS::AddConfigCallback addConfigFunc,
                              void* addConfigOpaque) {
        HDC dpy = getDummyDC(mDpy, WinDisplay::DEFAULT_DISPLAY);

        // wglChoosePixelFormat() needs to be called at least once,
        // i.e. it seems that the driver needs to initialize itself.
        // Do it here during initialization.
        initPixelFormat(dpy, mDispatch);

        // Quering number of formats
        PIXELFORMATDESCRIPTOR  pfd;
        int maxFormat = mDispatch->DescribePixelFormat(
                dpy, 1, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

        // Inserting rest of formats. Try to map each one to an EGL Config.
        for (int configId = 1; configId <= maxFormat; configId++) {
            mDispatch->DescribePixelFormat(
                    dpy, configId, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
            pixelFormatToConfig(
                    mDpy,
                    mDispatch,
                    renderableType,
                    &pfd,
                    configId,
                    addConfigFunc,
                    addConfigOpaque);
        }
    }

    virtual bool isValidNativeWin(EglOS::Surface* win) {
        if (!win) {
            return false;
        } else {
            return isValidNativeWin(WinSurface::from(win)->getHwnd());
        }
    }

    virtual bool isValidNativeWin(EGLNativeWindowType win) {
        return IsWindow(win);
    }

    virtual bool checkWindowPixelFormatMatch(
            EGLNativeWindowType win,
            const EglOS::PixelFormat* pixelFormat,
            unsigned int* width,
            unsigned int* height) {
        RECT r;
        if (!GetClientRect(win, &r)) {
            return false;
        }
        *width  = r.right  - r.left;
        *height = r.bottom - r.top;
        HDC dc = GetDC(win);
        const WinPixelFormat* format = WinPixelFormat::from(pixelFormat);
        bool ret = mDispatch->SetPixelFormat(dc,
                                             format->configId(),
                                             format->desc());
        DeleteDC(dc);
        return ret;
    }

    virtual EglOS::Context* createContext(
            const EglOS::PixelFormat* pixelFormat,
            EglOS::Context* sharedContext) {
        const WinPixelFormat* format = WinPixelFormat::from(pixelFormat);
        HDC dpy = getDummyDC(mDpy, format->configId());

        if (!mDpy->isPixelFormatSet(format->configId())) {
            if (!mDispatch->SetPixelFormat(dpy,
                                           format->configId(),
                                           format->desc())) {
                return NULL;
            }
            mDpy->pixelFormatWasSet(format->configId());
        }

        HGLRC ctx = mDispatch->wglCreateContext(dpy);

        if (ctx && sharedContext) {
            if (!mDispatch->wglShareLists(WinContext::from(sharedContext), ctx)) {
                mDispatch->wglDeleteContext(ctx);
                return NULL;
            }
        }
        return new WinContext(ctx);
    }

    virtual bool destroyContext(EglOS::Context* context) {
        if (!context) {
            return false;
        }
        if (!mDispatch->wglDeleteContext(WinContext::from(context))) {
            GetLastError();
            return false;
        }
        delete context;
        return true;
    }

    virtual EglOS::Surface* createPbufferSurface(
            const EglOS::PixelFormat* pixelFormat,
            const EglOS::PbufferInfo* info) {
        int configId = WinPixelFormat::from(pixelFormat)->configId();
        HDC dpy = getDummyDC(mDpy, configId);

        int wglTexFormat = WGL_NO_TEXTURE_ARB;
        int wglTexTarget =
                (info->target == EGL_TEXTURE_2D) ? WGL_TEXTURE_2D_ARB
                                                 : WGL_NO_TEXTURE_ARB;
        switch (info->format) {
        case EGL_TEXTURE_RGB:
            wglTexFormat = WGL_TEXTURE_RGB_ARB;
            break;
        case EGL_TEXTURE_RGBA:
            wglTexFormat = WGL_TEXTURE_RGBA_ARB;
            break;
        }

        const int pbAttribs[] = {
            WGL_TEXTURE_TARGET_ARB, wglTexTarget,
            WGL_TEXTURE_FORMAT_ARB, wglTexFormat,
            0
        };

        const WglExtensionsDispatch* dispatch = mDispatch;
        if (!dispatch->wglCreatePbufferARB) {
            return NULL;
        }
        HPBUFFERARB pb = dispatch->wglCreatePbufferARB(
                dpy, configId, info->width, info->height, pbAttribs);
        if (!pb) {
            GetLastError();
            return NULL;
        }
        return new WinSurface(pb, dispatch);
    }

    virtual bool releasePbuffer(EglOS::Surface* pb) {
        if (!pb) {
            return false;
        }
        const WglExtensionsDispatch* dispatch = mDispatch;
        if (!dispatch->wglReleasePbufferDCARB ||
            !dispatch->wglDestroyPbufferARB) {
            return false;
        }
        WinSurface* winpb = WinSurface::from(pb);
        if (!dispatch->wglReleasePbufferDCARB(
                winpb->getPbuffer(), winpb->getDC()) ||
            !dispatch->wglDestroyPbufferARB(winpb->getPbuffer())) {
            GetLastError();
            return false;
        }
        return true;
    }

    virtual bool makeCurrent(EglOS::Surface* read,
                             EglOS::Surface* draw,
                             EglOS::Context* context) {
        HDC hdcRead = read ? WinSurface::from(read)->getDC() : NULL;
        HDC hdcDraw = draw ? WinSurface::from(draw)->getDC() : NULL;
        HGLRC hdcContext = context ? WinContext::from(context) : 0;

        const WglExtensionsDispatch* dispatch = mDispatch;

        if (hdcRead == hdcDraw){
            // The following loop is a work-around for a problem when
            // occasionally the rendering is incorrect after hibernating and
            // waking up windows.
            // wglMakeCurrent will sometimes fail for a short period of time
            // in such situation.
            //
            // For a stricter test, in addition to checking the return value, we
            // might also want to check its error code. On my computer in such
            // situation GetLastError() returns 0 (which is documented as
            // success code). This is not a documented behaviour and is
            // unreliable. But in case one needs to use it, here is the code:
            //      while (!(isSuccess = dispatch->wglMakeCurrent(hdcDraw, hdcContext)) && GetLastError()==0) Sleep(1000);

            while (!dispatch->wglMakeCurrent(hdcDraw, hdcContext)) {
                Sleep(100);
            }
            return true;
        } else if (!dispatch->wglMakeContextCurrentARB) {
            return false;
        }
        bool retVal = dispatch->wglMakeContextCurrentARB(
                hdcDraw, hdcRead, hdcContext);
        return retVal;
    }

    virtual void swapBuffers(EglOS::Surface* srfc) {
        if (srfc && !mDispatch->SwapBuffers(WinSurface::from(srfc)->getDC())) {
            GetLastError();
        }
    }

private:
    WinDisplay* mDpy;
    const WglExtensionsDispatch* mDispatch;
};

class WglLibrary : public GlLibrary {
public:
    WglLibrary(const WglBaseDispatch* dispatch) : mDispatch(dispatch) {}

    virtual GlFunctionPointer findSymbol(const char* name) {
        return mDispatch->findFunction(name);
    }

private:
    const WglBaseDispatch* mDispatch;
};

class WinEngine : public EglOS::Engine {
public:
    WinEngine();

    ~WinEngine() {
        delete mDispatch;
        delete mLib;
    }

    virtual EglOS::Display* getDefaultDisplay() {
        WinDisplay* dpy = new WinDisplay();

        HWND hwnd = createDummyWindow();
        HDC  hdc  =  GetDC(hwnd);
        dpy->setInfo(WinDisplay::DEFAULT_DISPLAY, DisplayInfo(hdc, hwnd));

        return new WglDisplay(dpy, mDispatch);
    }

    virtual GlLibrary* getGlLibrary() {
        return &mGlLib;
    }

    virtual EglOS::Surface* createWindowSurface(EGLNativeWindowType wnd) {
        return new WinSurface(wnd);
    }

private:
    SharedLibrary* mLib;
    const WglExtensionsDispatch* mDispatch;
    WglBaseDispatch mBaseDispatch;
    WglLibrary mGlLib;
};

WinEngine::WinEngine() :
        mLib(NULL),
        mDispatch(NULL),
        mBaseDispatch(),
        mGlLib(&mBaseDispatch) {
    if (!s_tlsIndex) {
        s_tlsIndex = TlsAlloc();
    }
    const char* kLibName = "opengl32.dll";
    bool isSystemLib = true;
    const char* env = ::getenv("ANDROID_GL_LIB");
    if (env && !strcmp(env, "mesa")) {
        kLibName = "mesa_opengl32.dll";
        isSystemLib = false;
    }
    char error[256];
    D("%s: Trying to load %s\n", __FUNCTION__, kLibName);
    mLib = SharedLibrary::open(kLibName, error, sizeof(error));
    if (!mLib) {
        ERR("ERROR: %s: Could not open %s: %s\n", __FUNCTION__,
            kLibName, error);
        exit(1);
    }

    D("%s: Library loaded at %p\n", __FUNCTION__, mLib);
    mBaseDispatch.init(mLib, isSystemLib);
    mDispatch = initExtensionsDispatch(&mBaseDispatch);
    D("%s: Dispatch initialized\n", __FUNCTION__);
}

emugl::LazyInstance<WinEngine> sHostEngine = LAZY_INSTANCE_INIT;

}  // namespace

// static
EglOS::Engine* EglOS::Engine::getHostInstance() {
    return sHostEngine.ptr();
}
