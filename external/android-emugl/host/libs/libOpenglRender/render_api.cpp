/*
* Copyright (C) 2011-2015 The Android Open Source Project
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
#include "OpenglRender/render_api.h"

#include "IOStream.h"
#include "RenderServer.h"
#include "RenderWindow.h"
#include "TimeUtils.h"

#include "TcpStream.h"
#ifdef _WIN32
#include "Win32PipeStream.h"
#else
#include "UnixStream.h"
#endif

#include "DispatchTables.h"

#include "OpenGLESDispatch/EGLDispatch.h"
#include "OpenGLESDispatch/GLESv1Dispatch.h"
#include "OpenGLESDispatch/GLESv2Dispatch.h"

#include "emugl/common/crash_reporter.h"
#include "emugl/common/logging.h"

#include <string.h>

GLESv2Dispatch s_gles2;
GLESv1Dispatch s_gles1;
static RenderServer* s_renderThread = NULL;
static char s_renderAddr[256];

static RenderWindow* s_renderWindow = NULL;

static IOStream *createRenderThread(int p_stream_buffer_size,
                                    unsigned int clientFlags);

RENDER_APICALL int RENDER_APIENTRY initLibrary(void)
{
    //
    // Load EGL Plugin
    //
    if (!init_egl_dispatch()) {
        // Failed to load EGL
        printf("Failed to init_egl_dispatch\n");
        return false;
    }

    //
    // Load GLES Plugin
    //
    if (!gles1_dispatch_init(&s_gles1)) {
        // Failed to load GLES
        ERR("Failed to gles1_dispatch_init\n");
        return false;
    }

    /* failure to init the GLES2 dispatch table is not fatal */
    if (!gles2_dispatch_init(&s_gles2)) {
        ERR("Failed to gles2_dispatch_init\n");
        return false;
    }

    return true;
}

RENDER_APICALL int RENDER_APIENTRY initOpenGLRenderer(
        int width, int height, bool useSubWindow, char* addr, size_t addrLen,
        emugl_logger_struct logfuncs, emugl_crash_func_t crashfunc) {
    set_emugl_crash_reporter(crashfunc);
    set_emugl_logger(logfuncs.coarse);
    set_emugl_cxt_logger(logfuncs.fine);
    //
    // Fail if renderer is already initialized
    //
    if (s_renderThread) {
        return false;
    }

    // kUseThread is used to determine whether the RenderWindow should use
    // a separate thread to manage its subwindow GL/GLES context.
    // For now, this feature is disabled entirely for the following
    // reasons:
    //
    // - It must be disabled on Windows at all times, otherwise the main window becomes
    //   unresponsive after a few seconds of user interaction (e.g. trying to
    //   move it over the desktop). Probably due to the subtle issues around
    //   input on this platform (input-queue is global, message-queue is
    //   per-thread). Also, this messes considerably the display of the
    //   main window when running the executable under Wine.
    //
    // - On Linux/XGL and OSX/Cocoa, this used to be necessary to avoid corruption
    //   issues with the GL state of the main window when using the SDL UI.
    //   After the switch to Qt, this is no longer necessary and may actually cause
    //   undesired interactions between the UI thread and the RenderWindow thread:
    //   for example, in a multi-monitor setup the context might be recreated when
    //   dragging the window between monitors, triggering a Qt-specific callback
    //   in the context of RenderWindow thread, which will become blocked on the UI
    //   thread, which may in turn be blocked on something else.
    bool kUseThread = false;

    //
    // initialize the renderer and listen to connections
    // on a thread in the current process.
    //
    s_renderWindow = new RenderWindow(width, height, kUseThread, useSubWindow);
    if (!s_renderWindow) {
        ERR("Could not create rendering window class");
        GL_LOG("Could not create rendering window class");
        return false;
    }
    if (!s_renderWindow->isValid()) {
        ERR("Could not initialize emulated framebuffer\n");
        delete s_renderWindow;
        s_renderWindow = NULL;
        return false;
    }

    s_renderThread = RenderServer::create(addr, addrLen);
    if (!s_renderThread) {
        return false;
    }
    strncpy(s_renderAddr, addr, sizeof(s_renderAddr));

    s_renderThread->start();

    GL_LOG("OpenGL renderer initialized successfully");
    return true;
}

RENDER_APICALL void RENDER_APIENTRY setPostCallback(
        OnPostFn onPost, void* onPostContext) {
    if (s_renderWindow) {
        s_renderWindow->setPostCallback(onPost, onPostContext);
    } else {
        ERR("Calling setPostCallback() before creating render window!");
    }
}

RENDER_APICALL void RENDER_APIENTRY getHardwareStrings(
        const char** vendor,
        const char** renderer,
        const char** version) {
    if (s_renderWindow &&
        s_renderWindow->getHardwareStrings(vendor, renderer, version)) {
        return;
    }
    *vendor = *renderer = *version = NULL;
}

RENDER_APICALL int RENDER_APIENTRY stopOpenGLRenderer(void)
{
    bool ret = false;

    // open a dummy connection to the renderer to make it
    // realize the exit request.
    // (send the exit request in clientFlags)
    IOStream *dummy = createRenderThread(8, IOSTREAM_CLIENT_EXIT_SERVER);
    if (!dummy) return false;

    if (s_renderThread) {
        // wait for the thread to exit
        ret = s_renderThread->wait(NULL);

        delete s_renderThread;
        s_renderThread = NULL;
    }

    if (s_renderWindow != NULL) {
        delete s_renderWindow;
        s_renderWindow = NULL;
    }

    delete dummy;

    return ret;
}

RENDER_APICALL bool RENDER_APIENTRY showOpenGLSubwindow(
        FBNativeWindowType window_id,
        int wx,
        int wy,
        int ww,
        int wh,
        int fbw,
        int fbh,
        float dpr,
        float zRot)
{
    RenderWindow* window = s_renderWindow;

    if (window) {
       return window->setupSubWindow(window_id,wx,wy,ww,wh,fbw,fbh,dpr,zRot);
    }
    // XXX: should be implemented by sending the renderer process
    //      a request
    ERR("%s not implemented for separate renderer process !!!\n",
        __FUNCTION__);
    return false;
}

RENDER_APICALL bool RENDER_APIENTRY destroyOpenGLSubwindow(void)
{
    RenderWindow* window = s_renderWindow;

    if (window) {
        return window->removeSubWindow();
    }

    // XXX: should be implemented by sending the renderer process
    //      a request
    ERR("%s not implemented for separate renderer process !!!\n",
            __FUNCTION__);
    return false;
}

RENDER_APICALL void RENDER_APIENTRY setOpenGLDisplayRotation(float zRot)
{
    RenderWindow* window = s_renderWindow;

    if (window) {
        window->setRotation(zRot);
        return;
    }
    // XXX: should be implemented by sending the renderer process
    //      a request
    ERR("%s not implemented for separate renderer process !!!\n",
            __FUNCTION__);
}

RENDER_APICALL void RENDER_APIENTRY setOpenGLDisplayTranslation(float px, float py)
{
    RenderWindow* window = s_renderWindow;

    if (window) {
        window->setTranslation(px, py);
        return;
    }
    // XXX: should be implemented by sending the renderer process
    //      a request
    ERR("%s not implemented for separate renderer process !!!\n",
            __FUNCTION__);
}

RENDER_APICALL void RENDER_APIENTRY repaintOpenGLDisplay(void)
{
    RenderWindow* window = s_renderWindow;

    if (window) {
        window->repaint();
        return;
    }
    // XXX: should be implemented by sending the renderer process
    //      a request
    ERR("%s not implemented for separate renderer process !!!\n",
            __FUNCTION__);
}


/* NOTE: For now, always use TCP mode by default, until the emulator
 *        has been updated to support Unix and Win32 pipes
 */
#define  DEFAULT_STREAM_MODE  RENDER_API_STREAM_MODE_TCP

int gRendererStreamMode = DEFAULT_STREAM_MODE;

IOStream *createRenderThread(int p_stream_buffer_size, unsigned int clientFlags)
{
    SocketStream*  stream = NULL;

    if (gRendererStreamMode == RENDER_API_STREAM_MODE_TCP) {
        stream = new TcpStream(p_stream_buffer_size);
    } else {
#ifdef _WIN32
        stream = new Win32PipeStream(p_stream_buffer_size);
#else /* !_WIN32 */
        stream = new UnixStream(p_stream_buffer_size);
#endif
    }

    if (!stream) {
        ERR("createRenderThread failed to create stream\n");
        return NULL;
    }
    if (stream->connect(s_renderAddr) < 0) {
        ERR("createRenderThread failed to connect\n");
        delete stream;
        return NULL;
    }

    //
    // send clientFlags to the renderer
    //
    unsigned int *pClientFlags =
                (unsigned int *)stream->allocBuffer(sizeof(unsigned int));
    *pClientFlags = clientFlags;
    stream->commitBuffer(sizeof(unsigned int));

    return stream;
}

RENDER_APICALL int RENDER_APIENTRY setStreamMode(int mode)
{
    switch (mode) {
        case RENDER_API_STREAM_MODE_DEFAULT:
            mode = DEFAULT_STREAM_MODE;
            break;

        case RENDER_API_STREAM_MODE_TCP:
            break;

#ifndef _WIN32
        case RENDER_API_STREAM_MODE_UNIX:
            break;
#else /* _WIN32 */
        case RENDER_API_STREAM_MODE_PIPE:
            break;
#endif /* _WIN32 */
        default:
            // Invalid stream mode
            return false;
    }
    gRendererStreamMode = mode;
    return true;
}
