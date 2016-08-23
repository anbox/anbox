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

#include "MacNative.h"

#include "emugl/common/lazy_instance.h"
#include "emugl/common/shared_library.h"
#include "GLcommon/GLLibrary.h"
#include "OpenglCodecCommon/ErrorLog.h"

#include <list>

#define MAX_PBUFFER_MIPMAP_LEVEL 1

namespace {

class MacSurface : public EglOS::Surface {
public:
    MacSurface(void* handle, SurfaceType type) :
            Surface(type), m_handle(handle), m_hasMipmap(false) {}

    void* handle() const { return m_handle; }

    bool hasMipmap() const { return m_hasMipmap; }
    void setHasMipmap(bool value) { m_hasMipmap = value; }

    static MacSurface* from(EglOS::Surface* s) {
        return static_cast<MacSurface*>(s);
    }

private:
    void* m_handle;
    bool m_hasMipmap;
};

class MacContext : public EglOS::Context {
public:
    explicit MacContext(void* context) : mContext(context) {}

    virtual ~MacContext() {
        nsDestroyContext(mContext);
    }

    void* context() const { return mContext; }

    static void* from(EglOS::Context* c) {
        return static_cast<MacContext*>(c)->context();
    }

private:
    void* mContext;
};

typedef std::list<void*> NativeFormatList;

NativeFormatList s_nativeFormats;

void initNativeConfigs(){
    int nConfigs = getNumPixelFormats();
    if (s_nativeFormats.empty()) {
        for(int i = 0; i < nConfigs; i++) {
             void* frmt = getPixelFormat(i);
             if (frmt) {
                 s_nativeFormats.push_back(frmt);
             }
        }
    }
}

class MacPixelFormat : public EglOS::PixelFormat {
public:
    MacPixelFormat(void* handle, int redSize, int greenSize, int blueSize) :
            mHandle(handle),
            mRedSize(redSize),
            mGreenSize(greenSize),
            mBlueSize(blueSize) {}

    EglOS::PixelFormat* clone() {
        return new MacPixelFormat(mHandle, mRedSize, mGreenSize, mBlueSize);
    }

    void* handle() const { return mHandle; }
    int redSize() const { return mRedSize; }
    int greenSize() const { return mGreenSize; }
    int blueSize() const { return mBlueSize; }

    static const MacPixelFormat* from(const EglOS::PixelFormat* f) {
        return static_cast<const MacPixelFormat*>(f);
    }

private:
    MacPixelFormat();
    MacPixelFormat(const MacPixelFormat& other);

    void* mHandle;
    int mRedSize;
    int mGreenSize;
    int mBlueSize;
};


void pixelFormatToConfig(int index,
                         int renderableType,
                         void* frmt,
                         EglOS::AddConfigCallback addConfigFunc,
                         void* addConfigOpaque) {
    EglOS::ConfigInfo info;
    memset(&info, 0, sizeof(info));

    EGLint doubleBuffer;
    getPixelFormatAttrib(frmt, MAC_HAS_DOUBLE_BUFFER, &doubleBuffer);
    if (!doubleBuffer) {
        return; //pixel double buffer
    }

    EGLint window = 0, pbuffer = 0;
    getPixelFormatAttrib(frmt, MAC_DRAW_TO_WINDOW, &window);
    getPixelFormatAttrib(frmt, MAC_DRAW_TO_PBUFFER, &pbuffer);

    info.surface_type = 0;
    if (window) {
        info.surface_type |= EGL_WINDOW_BIT;
    }
    if (pbuffer) {
        info.surface_type |= EGL_PBUFFER_BIT;
    }
    if (!info.surface_type) {
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
    info.trans_red_val = 0;
    info.trans_green_val = 0;
    info.trans_blue_val = 0;

    info.transparent_type = EGL_NONE;

    /* All configs can end up having an alpha channel even if none was requested.
     * The default config chooser in GLSurfaceView will therefore not find any
     * matching config. Thus, make sure alpha is zero (or at least signalled as
     * zero to the calling EGL layer) for the configs where it was intended to
     * be zero. */
    if (getPixelFormatDefinitionAlpha(index) == 0) {
        info.alpha_size = 0;
    } else {
        getPixelFormatAttrib(frmt, MAC_ALPHA_SIZE, &info.alpha_size);
    }
    getPixelFormatAttrib(frmt, MAC_DEPTH_SIZE, &info.depth_size);
    getPixelFormatAttrib(frmt, MAC_STENCIL_SIZE, &info.stencil_size);
    getPixelFormatAttrib(frmt, MAC_SAMPLES_PER_PIXEL, &info.samples_per_pixel);

    //TODO: ask guy if it is OK
    GLint colorSize = 0;
    getPixelFormatAttrib(frmt, MAC_COLOR_SIZE, &colorSize);
    info.red_size = info.green_size = info.blue_size = (colorSize / 4);

    info.config_id = (EGLint) index;
    info.frmt = new MacPixelFormat(
            frmt, info.red_size, info.green_size, info.blue_size);

    (*addConfigFunc)(addConfigOpaque, &info);
}


class MacDisplay : public EglOS::Display {
public:
    explicit MacDisplay(EGLNativeDisplayType dpy) : mDpy(dpy) {}

    virtual bool release() {
        return true;
    }

    virtual void queryConfigs(int renderableType,
                              EglOS::AddConfigCallback* addConfigFunc,
                              void* addConfigOpaque) {
        initNativeConfigs();
        int i = 0;
        for (NativeFormatList::iterator it = s_nativeFormats.begin();
                it != s_nativeFormats.end();
                it++) {
            pixelFormatToConfig(i++,
                                renderableType,
                                *it,
                                addConfigFunc,
                                addConfigOpaque);
        }
    }

    virtual bool isValidNativeWin(EglOS::Surface* win) {
        if (win->type() != MacSurface::WINDOW) {
            return false;
        } else {
            return isValidNativeWin(MacSurface::from(win)->handle());
        }
    }

    virtual bool isValidNativeWin(EGLNativeWindowType win) {
        unsigned int width, height;
        return nsGetWinDims(win, &width, &height);
    }

    virtual bool checkWindowPixelFormatMatch(
            EGLNativeWindowType win,
            const EglOS::PixelFormat* pixelFormat,
            unsigned int* width,
            unsigned int* height) {
        bool ret = nsGetWinDims(win, width, height);

        const MacPixelFormat* format = MacPixelFormat::from(pixelFormat);
        int r = format->redSize();
        int g = format->greenSize();
        int b = format->blueSize();

        bool match = nsCheckColor(win, r + g + b);

        return ret && match;
    }

    virtual EglOS::Context* createContext(
            const EglOS::PixelFormat* pixelFormat,
            EglOS::Context* sharedContext) {
        void* macSharedContext =
                sharedContext ? MacContext::from(sharedContext) : NULL;
        return new MacContext(
                nsCreateContext(MacPixelFormat::from(pixelFormat)->handle(),
                                macSharedContext));
    }

    virtual bool destroyContext(EglOS::Context* context) {
        delete context;
        return true;
    }

    virtual EglOS::Surface* createPbufferSurface(
            const EglOS::PixelFormat* pixelFormat,
            const EglOS::PbufferInfo* info) {
        GLenum glTexFormat = GL_RGBA, glTexTarget = GL_TEXTURE_2D;
        switch (info->format) {
        case EGL_TEXTURE_RGB:
            glTexFormat = GL_RGB;
            break;
        case EGL_TEXTURE_RGBA:
            glTexFormat = GL_RGBA;
            break;
        }
        EGLint maxMipmap = info->hasMipmap ? MAX_PBUFFER_MIPMAP_LEVEL : 0;

        MacSurface* result = new MacSurface(
                nsCreatePBuffer(
                        glTexTarget,
                        glTexFormat,
                        maxMipmap,
                        info->width,
                        info->height),
                MacSurface::PBUFFER);

        result->setHasMipmap(info->hasMipmap);
        return result;
    }

    virtual bool releasePbuffer(EglOS::Surface* pb) {
        if (pb) {
            nsDestroyPBuffer(MacSurface::from(pb)->handle());
        }
        return true;
    }

    virtual bool makeCurrent(EglOS::Surface* read,
                             EglOS::Surface* draw,
                             EglOS::Context* ctx) {
        // check for unbind
        if (ctx == NULL && read == NULL && draw == NULL) {
            nsWindowMakeCurrent(NULL, NULL);
            return true;
        }
        else if (ctx == NULL || read == NULL || draw == NULL) {
            // error !
            return false;
        }

        //dont supporting diffrent read & draw surfaces on Mac
        if (read != draw) {
            return false;
        }
        switch (draw->type()) {
        case MacSurface::WINDOW:
            nsWindowMakeCurrent(MacContext::from(ctx),
                                MacSurface::from(draw)->handle());
            break;
        case MacSurface::PBUFFER:
        {
            MacSurface* macdraw = MacSurface::from(draw);
            int mipmapLevel = macdraw->hasMipmap() ? MAX_PBUFFER_MIPMAP_LEVEL : 0;
            nsPBufferMakeCurrent(MacContext::from(ctx),
                                 macdraw->handle(), mipmapLevel);
            break;
        }
        default:
            return false;
        }
        return true;
    }

    virtual void swapBuffers(EglOS::Surface* srfc) {
        nsSwapBuffers();
    }

    EGLNativeDisplayType dpy() const { return mDpy; }

private:
    EGLNativeDisplayType mDpy;
};

class MacGlLibrary : public GlLibrary {
public:
    MacGlLibrary() : mLib(NULL) {
        static const char kLibName[] =
                "/System/Library/Frameworks/OpenGL.framework/OpenGL";
        char error[256];
        mLib = emugl::SharedLibrary::open(kLibName, error, sizeof(error));
        if (!mLib) {
            ERR("%s: Could not open GL library %s [%s]\n",
                __FUNCTION__, kLibName, error);
        }
    }

    ~MacGlLibrary() {
        delete mLib;
    }

    // override
    virtual GlFunctionPointer findSymbol(const char* name) {
        if (!mLib) {
            return NULL;
        }
        return reinterpret_cast<GlFunctionPointer>(mLib->findSymbol(name));
    }

private:
    emugl::SharedLibrary* mLib;
};

class MacEngine : public EglOS::Engine {
public:
    MacEngine() : mGlLib() {}

    virtual EglOS::Display* getDefaultDisplay() {
        return new MacDisplay(0);
    }

    virtual GlLibrary* getGlLibrary() {
        return &mGlLib;
    }

    virtual EglOS::Surface* createWindowSurface(EGLNativeWindowType wnd) {
        return new MacSurface(wnd, MacSurface::WINDOW);
    }

private:
    MacGlLibrary mGlLib;
};

emugl::LazyInstance<MacEngine> sHostEngine = LAZY_INSTANCE_INIT;

}  // namespace


// static
EglOS::Engine* EglOS::Engine::getHostInstance() {
    return sHostEngine.ptr();
}
