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
#include "emugl/common/mutex.h"
#include "emugl/common/shared_library.h"
#include "GLcommon/GLLibrary.h"

#include "OpenglCodecCommon/ErrorLog.h"

#include <string.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

namespace {

typedef Display X11Display;

class ErrorHandler{
public:
    ErrorHandler(EGLNativeDisplayType dpy);
    ~ErrorHandler();
    int getLastError() const { return s_lastErrorCode; }

private:
    static int s_lastErrorCode;
    int (*m_oldErrorHandler)(Display *, XErrorEvent *);
    static emugl::Mutex s_lock;
    static int errorHandlerProc(EGLNativeDisplayType dpy,XErrorEvent* event);
};

// static
int ErrorHandler::s_lastErrorCode = 0;

// static
emugl::Mutex ErrorHandler::s_lock;

ErrorHandler::ErrorHandler(EGLNativeDisplayType dpy) {
   emugl::Mutex::AutoLock mutex(s_lock);
   XSync(dpy,False);
   s_lastErrorCode = 0;
   m_oldErrorHandler = XSetErrorHandler(errorHandlerProc);
}

ErrorHandler::~ErrorHandler() {
   emugl::Mutex::AutoLock mutex(s_lock);
   XSetErrorHandler(m_oldErrorHandler);
   s_lastErrorCode = 0;
}

int ErrorHandler::errorHandlerProc(EGLNativeDisplayType dpy,
                                   XErrorEvent* event) {
    s_lastErrorCode = event->error_code;
    return 0;
}

#define IS_SUCCESS(a) \
        do { if (a != Success) return 0; } while (0)

#define EXIT_IF_FALSE(a) \
        do { if (a != Success) return; } while (0)

// Implementation of EglOS::PixelFormat based on GLX.
class GlxPixelFormat : public EglOS::PixelFormat {
public:
    explicit GlxPixelFormat(GLXFBConfig fbconfig) : mFbConfig(fbconfig) {}

    virtual EglOS::PixelFormat* clone() {
        return new GlxPixelFormat(mFbConfig);
    }

    GLXFBConfig fbConfig() const { return mFbConfig; }

    static GLXFBConfig from(const EglOS::PixelFormat* f) {
        return static_cast<const GlxPixelFormat*>(f)->fbConfig();
    }

private:
    GLXFBConfig mFbConfig;
};

void pixelFormatToConfig(EGLNativeDisplayType dpy,
                         int renderableType,
                         GLXFBConfig frmt,
                         EglOS::AddConfigCallback* addConfigFunc,
                         void* addConfigOpaque) {
    EglOS::ConfigInfo info;
    int  tmp;

    memset(&info, 0, sizeof(info));

    EXIT_IF_FALSE(glXGetFBConfigAttrib(dpy, frmt, GLX_TRANSPARENT_TYPE, &tmp));
    if (tmp == GLX_TRANSPARENT_INDEX) {
        return; // not supporting transparent index
    } else if (tmp == GLX_NONE) {
        info.transparent_type = EGL_NONE;
        info.trans_red_val = 0;
        info.trans_green_val = 0;
        info.trans_blue_val = 0;
    } else {
        info.transparent_type = EGL_TRANSPARENT_RGB;

        EXIT_IF_FALSE(glXGetFBConfigAttrib(
                dpy, frmt, GLX_TRANSPARENT_RED_VALUE, &info.trans_red_val));
        EXIT_IF_FALSE(glXGetFBConfigAttrib(
                dpy, frmt, GLX_TRANSPARENT_GREEN_VALUE, &info.trans_green_val));
        EXIT_IF_FALSE(glXGetFBConfigAttrib(
                dpy, frmt, GLX_TRANSPARENT_BLUE_VALUE, &info.trans_blue_val));
    }

    //
    // filter out single buffer configurations
    //
    int doubleBuffer = 0;
    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy, frmt, GLX_DOUBLEBUFFER, &doubleBuffer));
    if (!doubleBuffer) {
        return;
    }

    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy ,frmt, GLX_RED_SIZE, &info.red_size));
    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy ,frmt, GLX_GREEN_SIZE, &info.green_size));
    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy ,frmt, GLX_BLUE_SIZE, &info.blue_size));
    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy ,frmt, GLX_ALPHA_SIZE, &info.alpha_size));
    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy ,frmt, GLX_DEPTH_SIZE, &info.depth_size));
    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy ,frmt, GLX_STENCIL_SIZE, &info.stencil_size));

    info.renderable_type = renderableType;
    int nativeRenderable = 0;
    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy, frmt, GLX_X_RENDERABLE, &nativeRenderable));
    info.native_renderable = !!nativeRenderable;

    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy, frmt, GLX_X_VISUAL_TYPE, &info.native_visual_type));

    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy, frmt, GLX_VISUAL_ID, &info.native_visual_id));

    //supported surfaces types
    info.surface_type = 0;
    EXIT_IF_FALSE(glXGetFBConfigAttrib(dpy, frmt, GLX_DRAWABLE_TYPE, &tmp));
    if (tmp & GLX_WINDOW_BIT && info.native_visual_id != 0) {
        info.surface_type |= EGL_WINDOW_BIT;
    } else {
        info.native_visual_id = 0;
        info.native_visual_type = EGL_NONE;
    }
    if (tmp & GLX_PBUFFER_BIT) {
        info.surface_type |= EGL_PBUFFER_BIT;
    }

    info.caveat = 0;
    EXIT_IF_FALSE(glXGetFBConfigAttrib(dpy, frmt, GLX_CONFIG_CAVEAT, &tmp));
    if (tmp == GLX_NONE) {
        info.caveat = EGL_NONE;
    } else if (tmp == GLX_SLOW_CONFIG) {
        info.caveat = EGL_SLOW_CONFIG;
    } else if (tmp == GLX_NON_CONFORMANT_CONFIG) {
        info.caveat = EGL_NON_CONFORMANT_CONFIG;
    }
    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy, frmt, GLX_MAX_PBUFFER_WIDTH, &info.max_pbuffer_width));
    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy, frmt, GLX_MAX_PBUFFER_HEIGHT, &info.max_pbuffer_height));
    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy, frmt, GLX_MAX_PBUFFER_HEIGHT, &info.max_pbuffer_size));

    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy, frmt, GLX_LEVEL, &info.frame_buffer_level));

    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy, frmt, GLX_FBCONFIG_ID, &info.config_id));

    EXIT_IF_FALSE(glXGetFBConfigAttrib(
            dpy, frmt, GLX_SAMPLES, &info.samples_per_pixel));

    //Filter out configs that do not support RGBA
    EXIT_IF_FALSE(glXGetFBConfigAttrib(dpy, frmt, GLX_RENDER_TYPE, &tmp));
    if (!(tmp & GLX_RGBA_BIT)) {
        return;
    }

    info.frmt = new GlxPixelFormat(frmt);

    (*addConfigFunc)(addConfigOpaque, &info);
}

// Implementation of EglOS::Surface based on GLX.
class GlxSurface : public EglOS::Surface {
public:
    GlxSurface(GLXDrawable drawable, SurfaceType type) :
            Surface(type), mDrawable(drawable) {}

    GLXDrawable drawable() const { return mDrawable; }

    // Helper routine to down-cast an EglOS::Surface and extract
    // its drawable.
    static GLXDrawable drawableFor(EglOS::Surface* surface) {
        return static_cast<GlxSurface*>(surface)->drawable();
    }

private:
    GLXDrawable mDrawable;
};

// Implementation of EglOS::Context based on GLX.
class GlxContext : public EglOS::Context {
public:
    explicit GlxContext(GLXContext context) : mContext(context) {}

    GLXContext context() const { return mContext; }

    static GLXContext contextFor(EglOS::Context* context) {
        return static_cast<GlxContext*>(context)->context();
    }
private:
    GLXContext mContext;
};

// Implementation of EglOS::Display based on GLX.
class GlxDisplay : public EglOS::Display {
public:
    explicit GlxDisplay(X11Display* disp) : mDisplay(disp) {}

    virtual bool release() {
        return XCloseDisplay(mDisplay);
    }

    virtual void queryConfigs(int renderableType,
                              EglOS::AddConfigCallback* addConfigFunc,
                              void* addConfigOpaque) {
        int n;
        GLXFBConfig* frmtList = glXGetFBConfigs(mDisplay, 0, &n);
        for(int i = 0; i < n; i++) {
            pixelFormatToConfig(
                    mDisplay,
                    renderableType,
                    frmtList[i],
                    addConfigFunc,
                    addConfigOpaque);
        }
        XFree(frmtList);
    }

    virtual bool isValidNativeWin(EglOS::Surface* win) {
        if (!win) {
            return false;
        } else {
            return isValidNativeWin(GlxSurface::drawableFor(win));
        }
    }

    virtual bool isValidNativeWin(EGLNativeWindowType win) {
        Window root;
        int t;
        unsigned int u;
        ErrorHandler handler(mDisplay);
        if (!XGetGeometry(mDisplay, win, &root, &t, &t, &u, &u, &u, &u)) {
            return false;
        }
        return handler.getLastError() == 0;
    }

    virtual bool checkWindowPixelFormatMatch(
            EGLNativeWindowType win,
            const EglOS::PixelFormat* pixelFormat,
            unsigned int* width,
            unsigned int* height) {
        //TODO: to check what does ATI & NVIDIA enforce on win pixelformat
        unsigned int depth, configDepth, border;
        int r, g, b, x, y;
        GLXFBConfig fbconfig = GlxPixelFormat::from(pixelFormat);

        IS_SUCCESS(glXGetFBConfigAttrib(
                mDisplay, fbconfig, GLX_RED_SIZE, &r));
        IS_SUCCESS(glXGetFBConfigAttrib(
                mDisplay, fbconfig, GLX_GREEN_SIZE, &g));
        IS_SUCCESS(glXGetFBConfigAttrib(
                mDisplay, fbconfig, GLX_BLUE_SIZE, &b));
        configDepth = r + g + b;
        Window root;
        if (!XGetGeometry(
                mDisplay, win, &root, &x, &y, width, height, &border, &depth)) {
            return false;
        }
        return depth >= configDepth;
    }

    virtual EglOS::Context* createContext(
            const EglOS::PixelFormat* pixelFormat,
            EglOS::Context* sharedContext) {
        ErrorHandler handler(mDisplay);
        GLXContext ctx = glXCreateNewContext(
                mDisplay,
                GlxPixelFormat::from(pixelFormat),
                GLX_RGBA_TYPE,
                sharedContext ? GlxContext::contextFor(sharedContext) : NULL,
                true);

        if (handler.getLastError()) {
            return NULL;
        }

        return new GlxContext(ctx);
    }

    virtual bool destroyContext(EglOS::Context* context) {
        glXDestroyContext(mDisplay, GlxContext::contextFor(context));
        return true;
    }

    virtual EglOS::Surface* createPbufferSurface(
            const EglOS::PixelFormat* pixelFormat,
            const EglOS::PbufferInfo* info) {
        const int attribs[] = {
            GLX_PBUFFER_WIDTH, info->width,
            GLX_PBUFFER_HEIGHT, info->height,
            GLX_LARGEST_PBUFFER, info->largest,
            None
        };
        GLXPbuffer pb = glXCreatePbuffer(
                mDisplay,
                GlxPixelFormat::from(pixelFormat),
                attribs);
        return pb ? new GlxSurface(pb, GlxSurface::PBUFFER) : NULL;
    }

    virtual bool releasePbuffer(EglOS::Surface* pb) {
        if (!pb) {
            return false;
        } else {
            glXDestroyPbuffer(mDisplay, GlxSurface::drawableFor(pb));
            return true;
        }
    }

    virtual bool makeCurrent(EglOS::Surface* read,
                             EglOS::Surface* draw,
                             EglOS::Context* context) {
        ErrorHandler handler(mDisplay);
        bool retval = false;
        if (!context && !read && !draw) {
            // unbind
            retval = glXMakeContextCurrent(mDisplay, 0, 0, NULL);
        }
        else if (context && read && draw) {
            retval = glXMakeContextCurrent(
                    mDisplay,
                    GlxSurface::drawableFor(draw),
                    GlxSurface::drawableFor(read),
                    GlxContext::contextFor(context));
        }
        return (handler.getLastError() == 0) && retval;
    }

    virtual void swapBuffers(EglOS::Surface* srfc) {
        if (srfc) {
            glXSwapBuffers(mDisplay, GlxSurface::drawableFor(srfc));
        }
    }

private:
    X11Display* mDisplay;
};

class GlxLibrary : public GlLibrary {
public:
    typedef GlFunctionPointer (ResolverFunc)(const char* name);

    // Important: Use libGL.so.1 explicitly, because it will always link to
    // the vendor-specific version of the library. libGL.so might in some
    // cases, depending on bad ldconfig configurations, link to the wrapper
    // lib that doesn't behave the same.
    GlxLibrary() : mLib(NULL), mResolver(NULL) {
        static const char kLibName[] = "libGL.so.1";
        char error[256];
        mLib = emugl::SharedLibrary::open(kLibName, error, sizeof(error));
        if (!mLib) {
            ERR("%s: Could not open GL library %s [%s]\n",
                __FUNCTION__, kLibName, error);
            return;
        }
        // NOTE: Don't use glXGetProcAddress here, only glXGetProcAddressARB
        // is guaranteed to be supported by vendor-specific libraries.
        static const char kResolverName[] = "glXGetProcAddressARB";
        mResolver = reinterpret_cast<ResolverFunc*>(
                mLib->findSymbol(kResolverName));
        if (!mResolver) {
            ERR("%s: Could not find resolver %s in %s\n",
                __FUNCTION__, kResolverName, kLibName);
            delete mLib;
            mLib = NULL;
        }
    }

    ~GlxLibrary() {
        delete mLib;
    }

    // override
    virtual GlFunctionPointer findSymbol(const char* name) {
        if (!mLib) {
            return NULL;
        }
        GlFunctionPointer ret = (*mResolver)(name);
        if (!ret) {
            ret = reinterpret_cast<GlFunctionPointer>(mLib->findSymbol(name));
        }
        return ret;
    }

private:
    emugl::SharedLibrary* mLib;
    ResolverFunc* mResolver;
};

class GlxEngine : public EglOS::Engine {
public:
    GlxEngine() : mGlLib() {}

    virtual EglOS::Display* getDefaultDisplay() {
        return new GlxDisplay(XOpenDisplay(0));
    }

    virtual GlLibrary* getGlLibrary() {
        return &mGlLib;
    }

    virtual EglOS::Surface* createWindowSurface(EGLNativeWindowType wnd) {
        return new GlxSurface(wnd, GlxSurface::WINDOW);
    }
private:
    GlxLibrary mGlLib;
};

emugl::LazyInstance<GlxEngine> sHostEngine = LAZY_INSTANCE_INIT;

}  // namespace

// static
EglOS::Engine* EglOS::Engine::getHostInstance() {
    return sHostEngine.ptr();
}
