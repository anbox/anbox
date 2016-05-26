set(SOURCES
    ColorBuffer.cpp
    FbConfig.cpp
    FrameBuffer.cpp
    NativeSubWindow_mir.cpp
    ReadBuffer.cpp
    RenderContext.cpp
    RenderControl.cpp
    RenderServer.cpp
    RenderThread.cpp
    RenderThreadInfo.cpp
    render_api.cpp
    RenderWindow.cpp
    SocketStream.cpp
    TcpStream.cpp
    TextureDraw.cpp
    TextureResize.cpp
    TimeUtils.cpp
    UnixStream.cpp
    WindowSurface.cpp)

include_directories(BEFORE
    ${MIRCLIENT_INCLUDE_DIRS})

add_library(OpenglRender ${SOURCES})
target_link_libraries(OpenglRender
    emugl_common
    GLESv1_dec
    GLESv2_dec
    renderControl_dec
    OpenGLESDispatch
    OpenglCodecCommon
    mir_support
    ${EGL_LDFLAGS}
    ${EGL_LIBRARIES}
    ${MIRCLIENT_LDFLAGS}
    ${MIRCLIENT_LIBRARIES})
