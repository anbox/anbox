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

#define MIR_EGL_PLATFORM

#include "mir_support/shared_state.h"

#include "EglOsApi.h"

#include "emugl/common/lazy_instance.h"
#include "emugl/common/shared_library.h"
#include "GLcommon/GLLibrary.h"

#include "OpenglCodecCommon/ErrorLog.h"

#include <mirclient/mir_toolkit/mir_client_library.h>

#include <boost/throw_exception.hpp>
#include <stdexcept>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#define EXIT_IF_FALSE(a) \
        do { if (!a) return; } while (0)

namespace {
class HostEglImpl {
public:
    static HostEglImpl* get() {
        static HostEglImpl *inst = new HostEglImpl;
        return inst;
    }

    EGLint eglGetError() {
        typedef EGLint (*_eglGetErrorFunc)();
        static _eglGetErrorFunc _eglGetError;

        if (!_eglGetError)
            _eglGetError = reinterpret_cast<_eglGetErrorFunc>(lib_->findSymbol("eglGetError"));

        return _eglGetError();
    }

    EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id) {
        typedef EGLDisplay (*_eglGetDisplayFunc)(EGLNativeDisplayType display_id);
        static _eglGetDisplayFunc _eglGetDisplay;

        if (!_eglGetDisplay)
            _eglGetDisplay = reinterpret_cast<_eglGetDisplayFunc>(lib_->findSymbol("eglGetDisplay"));

        return _eglGetDisplay(display_id);
    }

    EGLBoolean eglInitialize(EGLDisplay display, EGLint *major, EGLint *minor) {
        typedef EGLBoolean (*_eglInitializeFunc)(EGLDisplay display, EGLint *major, EGLint *minor);
        static _eglInitializeFunc _eglInitialize = nullptr;

        if (!_eglInitialize)
            _eglInitialize = reinterpret_cast<_eglInitializeFunc>(lib_->findSymbol("eglInitialize"));

        return _eglInitialize(display, major, minor);
    }

    EGLBoolean eglTerminate(EGLDisplay display) {
        typedef EGLBoolean (*_eglTermianteFunc)(EGLDisplay display);
        static _eglTermianteFunc _eglTerminate = nullptr;

        if (!_eglTerminate)
            _eglTerminate = reinterpret_cast<_eglTermianteFunc>(lib_->findSymbol("eglTerminate"));

        return _eglTerminate(display);
    }

    EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) {
        typedef EGLContext (*_eglCreateContextFunc)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
        static _eglCreateContextFunc _eglCreateContext = nullptr;

        if (!_eglCreateContext)
            _eglCreateContext = reinterpret_cast<_eglCreateContextFunc>(lib_->findSymbol("eglCreateContext"));

        return _eglCreateContext(dpy, config, share_context, attrib_list);
    }

    EGLBoolean eglMakeCurrent(EGLDisplay display, EGLSurface read, EGLSurface draw, EGLContext context) {
        typedef EGLBoolean (*_eglMakeCurrentFunc)(EGLDisplay display, EGLSurface read, EGLSurface draw, EGLContext context);
        static _eglMakeCurrentFunc _eglMakeCurrent = nullptr;

        if (!_eglMakeCurrent)
            _eglMakeCurrent = reinterpret_cast<_eglMakeCurrentFunc>(lib_->findSymbol("eglMakeCurrent"));

        return _eglMakeCurrent(display, read, draw, context);
    }

    EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
        typedef EGLBoolean (*_eglGetConfigsFunc)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
        static _eglGetConfigsFunc _eglGetConfigs = nullptr;

        if (!_eglGetConfigs)
            _eglGetConfigs = reinterpret_cast<_eglGetConfigsFunc>(lib_->findSymbol("eglGetConfigs"));

        return _eglGetConfigs(dpy, configs, config_size, num_config);
    }

    EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
        typedef EGLSurface (*_eglCreatePbufferSurfaceFunc)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
        static _eglCreatePbufferSurfaceFunc _eglCreatePbufferSurface = nullptr;

        if (!_eglCreatePbufferSurface)
            _eglCreatePbufferSurface = reinterpret_cast<_eglCreatePbufferSurfaceFunc>(lib_->findSymbol("eglCreatePbufferSurface"));

        return _eglCreatePbufferSurface(dpy, config, attrib_list);
    }

    EGLBoolean eglDestroySurface(EGLDisplay display, EGLSurface surface) {
        typedef EGLBoolean (*_eglDestroySurfaceFunc)(EGLDisplay display, EGLSurface surface);
        static _eglDestroySurfaceFunc _eglDestroySurface = nullptr;

        if (!_eglDestroySurface)
            _eglDestroySurface = reinterpret_cast<_eglDestroySurfaceFunc>(lib_->findSymbol("eglDestroySurface"));

        return _eglDestroySurface(display, surface);
    }

    EGLBoolean eglSwapBuffers(EGLDisplay display, EGLSurface surface) {
        typedef EGLBoolean (*_eglSwapBuffersFunc)(EGLDisplay display, EGLSurface surface);
        static _eglSwapBuffersFunc _eglSwapBuffers = nullptr;

        if (!_eglSwapBuffers)
            _eglSwapBuffers = reinterpret_cast<_eglSwapBuffersFunc>(lib_->findSymbol("eglSwapBuffers"));

        return _eglSwapBuffers(display, surface);
    }

    EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) {
        typedef EGLBoolean (*_eglGetConfigAttribFunc)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
        static _eglGetConfigAttribFunc _eglGetConfigAttrib = nullptr;

        if (!_eglGetConfigAttrib)
            _eglGetConfigAttrib = reinterpret_cast<_eglGetConfigAttribFunc>(lib_->findSymbol("eglGetConfigAttrib"));

        return _eglGetConfigAttrib(dpy, config, attribute, value);
    }

    EGLSurface eglCreateWindowSurface(EGLDisplay display, EGLConfig config, EGLNativeWindowType window, const EGLint *attribs) {
        typedef EGLSurface (*_eglCreateWindowSurfaceFunc)(EGLDisplay display, EGLConfig config, EGLNativeWindowType window, const EGLint *attribs);
        static _eglCreateWindowSurfaceFunc _eglCreateWindowSurface;

        if (!_eglCreateWindowSurface)
            _eglCreateWindowSurface = reinterpret_cast<_eglCreateWindowSurfaceFunc>(lib_->findSymbol("eglCreateWindowSurface"));

        return _eglCreateWindowSurface(display, config, window, attribs);
    }

private:
    HostEglImpl() {
        static const char kLibName[] = "libEGL.so.1";

        char error[256];
        lib_ = emugl::SharedLibrary::open(kLibName, error, sizeof(error));
        if (!lib_) {
            ERR("%s: Could not open host EGL library %s [%s]\n",
                __FUNCTION__, kLibName, error);
            return;
        }
    }

    emugl::SharedLibrary *lib_;
};

class MirImplPixelFormat : public EglOS::PixelFormat {
public:
    MirImplPixelFormat(EGLConfig config) :
        config_(config) {
    }

    EGLConfig nativeFormat() const { return config_; }

    virtual EglOS::PixelFormat* clone() {
        return new MirImplPixelFormat(config_);
    }

    static const MirImplPixelFormat* from(const EglOS::PixelFormat *format) {
        return static_cast<const MirImplPixelFormat*>(format);
    }

    EGLConfig config() const {
        return config_;
    }

private:
    EGLConfig config_;
};

class MirImplSurface : public EglOS::Surface {
public:
    MirImplSurface(SurfaceType type) :
        Surface(type),
        surface_(EGL_NO_SURFACE) {
    }

    static MirImplSurface* create_with_surface(EGLSurface surface) {
        auto impl = new MirImplSurface(SurfaceType::PBUFFER);
        impl->surface_ = surface;
        return impl;
    }

    static MirImplSurface* create_with_native_window(EGLSurface surface) {
        auto impl = new MirImplSurface(SurfaceType::WINDOW);
        impl->surface_ = surface;
        return impl;
    }

    EGLSurface drawable() {
        return surface_;
    }

    void invalidate() {
        surface_ = EGL_NO_SURFACE;
    }

    static MirImplSurface* from(EglOS::Surface *surface) {
        if (!surface)
            return nullptr;

        return static_cast<MirImplSurface*>(surface);
    }

private:
    EGLSurface surface_;
    EGLNativeWindowType native_window_;
};

class MirImplContext : public EglOS::Context {
public:
    MirImplContext(EGLDisplay display, const MirImplPixelFormat *format, MirImplContext *shared_context) :
        egl_display_(display),
        egl_context_(EGL_NO_CONTEXT) {

        EGLint context_attribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };

        egl_context_ = HostEglImpl::get()->eglCreateContext(egl_display_,
                                        format ? format->nativeFormat() : nullptr,
                                        shared_context ? shared_context->nativeContext() : EGL_NO_CONTEXT,
                                        context_attribs);
        if (egl_context_ == EGL_NO_CONTEXT)
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create EGL context"));
    }

    bool release() {
        if (!eglDestroyContext(egl_display_, egl_context_))
            return false;

        egl_display_ = EGL_NO_DISPLAY;
        egl_context_ = EGL_NO_CONTEXT;

        return true;
    }

    EGLContext nativeContext() const {
        return egl_context_;
    }

    static MirImplContext* from(EglOS::Context *context) {
        if (!context)
            return nullptr;

        return static_cast<MirImplContext*>(context);
    }

private:
    EGLDisplay egl_display_;
    EGLContext egl_context_;
};

class MirImplDisplay : public EglOS::Display {
public:
    explicit MirImplDisplay(MirConnection *connection) :
        connection_(connection) {
        if (!mir_connection_is_valid(connection_))
            BOOST_THROW_EXCEPTION(std::runtime_error("Not connected to Mir server"));

        egl_display_ = HostEglImpl::get()->eglGetDisplay(mir_connection_get_egl_native_display(connection_));
        if (egl_display_ == EGL_NO_DISPLAY)
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to get EGL display"));

        if (!HostEglImpl::get()->eglInitialize(egl_display_, nullptr, nullptr))
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize EGL"));
    }

    virtual bool release() override {
        HostEglImpl::get()->eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        HostEglImpl::get()->eglTerminate(egl_display_);
        egl_display_ = EGL_NO_DISPLAY;
        return true;
    }

    virtual void queryConfigs(int renderable_type, EglOS::AddConfigCallback *callback, void *user_data) override {
        int num_configs = 0;
        HostEglImpl::get()->eglGetConfigs(egl_display_, nullptr, 0, &num_configs);

        auto configs = (EGLConfig*) calloc(num_configs, sizeof(EGLConfig));
        int num_config_available = 0;
        HostEglImpl::get()->eglGetConfigs(egl_display_, configs, num_configs, &num_config_available);

        for (int n = 0; n < num_config_available; n++) {
            EGLConfig config = configs[n];

            EglOS::ConfigInfo info;
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_RED_SIZE, &info.red_size));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_GREEN_SIZE, &info.green_size));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_BLUE_SIZE, &info.blue_size));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_ALPHA_SIZE, &info.alpha_size));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_CONFIG_CAVEAT, &info.caveat));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_CONFIG_ID, &info.config_id));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_DEPTH_SIZE, &info.depth_size));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_LEVEL, &info.frame_buffer_level));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_MAX_PBUFFER_WIDTH, &info.max_pbuffer_width));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_MAX_PBUFFER_HEIGHT, &info.max_pbuffer_height));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_MAX_PBUFFER_PIXELS, &info.max_pbuffer_size));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_NATIVE_RENDERABLE, &info.native_renderable));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_RENDERABLE_TYPE, &info.renderable_type));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_NATIVE_VISUAL_ID, &info.native_visual_id));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_NATIVE_VISUAL_TYPE, &info.native_visual_type));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_SAMPLES, &info.samples_per_pixel));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_STENCIL_SIZE, &info.stencil_size));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_SURFACE_TYPE, &info.surface_type));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_TRANSPARENT_TYPE, &info.transparent_type));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_TRANSPARENT_RED_VALUE, &info.trans_red_val));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_TRANSPARENT_GREEN_VALUE, &info.trans_green_val));
            EXIT_IF_FALSE(copy_egl_attrib(config, EGL_TRANSPARENT_BLUE_VALUE, &info.trans_blue_val));

            info.frmt = new MirImplPixelFormat(config);

            (*callback)(user_data, &info);
        }

        free(configs);
    }

    virtual bool isValidNativeWin(EglOS::Surface *surface) override {
        if (!surface)
            return false;

        return surface->type() == EglOS::Surface::WINDOW;
    }

    virtual bool isValidNativeWin(EGLNativeWindowType window) override {
        return true;
    }

    virtual bool checkWindowPixelFormatMatch(
                    EGLNativeWindowType window,
                    const EglOS::PixelFormat *format,
                    unsigned int *width,
                    unsigned int *height) override {

        auto impl = MirImplPixelFormat::from(format);

        EGLint r = 0, g = 0, b = 0;
        if (!copy_egl_attrib(impl->nativeFormat(), EGL_RED_SIZE, &r))
            return false;
        if (!copy_egl_attrib(impl->nativeFormat(), EGL_GREEN_SIZE, &g))
            return false;
        if (!copy_egl_attrib(impl->nativeFormat(), EGL_BLUE_SIZE, &b))
            return false;

        const auto config_depth = r + g +b;

        return true;
    }

    virtual EglOS::Context* createContext(const EglOS::PixelFormat *format, EglOS::Context *shared_context) override {
        try {
            return new MirImplContext(egl_display_,
                                      MirImplPixelFormat::from(format),
                                      MirImplContext::from(shared_context));
        }
        catch (std::exception &err) {
            ERR("Failed to create EGL context: %s\n", err.what());
            return nullptr;
        }
    }

    virtual bool destroyContext(EglOS::Context *context) override {
        if (!context)
            return context;

        return MirImplContext::from(context)->release();
    }

    virtual EglOS::Surface* createPbufferSurface(
                const EglOS::PixelFormat* format,
                const EglOS::PbufferInfo* info) override {

        EGLint attribs[] = {
            EGL_WIDTH, info->width,
            EGL_HEIGHT, info->height,
            EGL_LARGEST_PBUFFER, info->largest,
            EGL_MIPMAP_TEXTURE, info->hasMipmap,
            EGL_TEXTURE_TARGET, info->target,
            EGL_NONE,
        };

        auto surface = HostEglImpl::get()->eglCreatePbufferSurface(egl_display_,
                                               MirImplPixelFormat::from(format)->nativeFormat(),
                                               attribs);
        if (surface == EGL_NO_SURFACE)
            return nullptr;

        return MirImplSurface::create_with_surface(surface);
    }

    virtual bool releasePbuffer(EglOS::Surface *surface) override {
        if (!surface)
            return false;

        auto impl = MirImplSurface::from(surface);
        if (!impl)
            return false;

        if (!HostEglImpl::get()->eglDestroySurface(egl_display_, impl->drawable()))
            return false;

        impl->invalidate();
        return true;
    }

    virtual bool makeCurrent(EglOS::Surface* input_surface,
                EglOS::Surface* output_surface,
                EglOS::Context* context) override {

        bool success = false;

        if (!input_surface && !output_surface && !context) {
            success = HostEglImpl::get()->eglMakeCurrent(egl_display_,
                                     EGL_NO_SURFACE,
                                     EGL_NO_SURFACE,
                                     EGL_NO_CONTEXT);
        }
        else if (input_surface && output_surface && context) {
            success = HostEglImpl::get()->eglMakeCurrent(egl_display_,
                                     MirImplSurface::from(input_surface)->drawable(),
                                     MirImplSurface::from(output_surface)->drawable(),
                                     MirImplContext::from(context)->nativeContext());
        }

        return success;
    }

    virtual void swapBuffers(EglOS::Surface *surface) override {
        HostEglImpl::get()->eglSwapBuffers(egl_display_, MirImplSurface::from(surface)->drawable());
    }

    EGLDisplay display() const {
        return egl_display_;
    }

private:
    template<typename T>
    bool copy_egl_attrib(EGLConfig config, EGLint attribute, T *target) {
        T value;

        if (!HostEglImpl::get()->eglGetConfigAttrib(egl_display_, config, attribute, reinterpret_cast<EGLint*>(&value)))
            return false;

        *target = value;

        return true;
    }

    MirConnection *connection_;
    EGLDisplay egl_display_;
};

class MirImplGlLibrary : public GlLibrary {
public:
    typedef GlFunctionPointer (ResolverFunc)(const char* name);

    MirImplGlLibrary() {
        static const char kLibName[] = "libGLESv2.so.2";
        static const char kEGLLibName[] = "libEGL.so.1";

        char error[256];
        lib_ = emugl::SharedLibrary::open(kLibName, error, sizeof(error));
        if (!lib_) {
            ERR("%s: Could not open host GLESv2 library %s [%s]\n",
                __FUNCTION__, kLibName, error);
            return;
        }

        egl_lib_ = emugl::SharedLibrary::open(kEGLLibName, error, sizeof(error));
        if (!lib_) {
            ERR("%s: Could not open host EGL library %s [%s]\n",
                __FUNCTION__, kEGLLibName, error);
            return;
        }

        resolver_ = reinterpret_cast<ResolverFunc*>(egl_lib_->findSymbol("eglGetProcAddress"));
    }

    virtual GlFunctionPointer findSymbol(const char *name) override {
        if (!lib_)
            return nullptr;

        ERR("Trying to resolved %s ...\n", name);

        GlFunctionPointer symbol = nullptr;

        if (resolver_)
            symbol = (*resolver_)(name);

        if (!symbol)
            symbol = reinterpret_cast<GlFunctionPointer>(lib_->findSymbol(name));

        ERR("-> %p\n", symbol);

        return symbol;
    }

private:
    emugl::SharedLibrary *lib_;
    emugl::SharedLibrary *egl_lib_;
    ResolverFunc *resolver_;
};

class MirImplEngine : public EglOS::Engine {
public:
    MirImplEngine() {
        ERR("Initialized Mir EGL engine");
    }

    ~MirImplEngine() {
    }

    virtual EglOS::Display* getDefaultDisplay() override {
        mir::support::SharedState::get()->ensure_connection();

        return new MirImplDisplay(mir::support::SharedState::get()->connection());
    }

    virtual GlLibrary* getGlLibrary() {
        return &lib_;
    }

    virtual EglOS::Surface* createWindowSurface(EglOS::Display *display, EGLNativeWindowType window, EglOS::PixelFormat *format) override {
        mir::support::SharedState::get()->ensure_connection();

        auto surface = HostEglImpl::get()->eglCreateWindowSurface(
                    static_cast<MirImplDisplay*>(display)->display(),
                    static_cast<MirImplPixelFormat*>(format)->config(),
                    window,
                    nullptr);

        return MirImplSurface::create_with_native_window(surface);
    }

private:
    MirImplGlLibrary lib_;
};

emugl::LazyInstance<MirImplEngine> sHostEngine = LAZY_INSTANCE_INIT;
}

EglOS::Engine* EglOS::Engine::getHostInstance() {
    return sHostEngine.ptr();
}
