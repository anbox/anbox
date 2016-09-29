set(SOURCES
    ThreadInfo.cpp
    EglImp.cpp
    EglConfig.cpp
    EglContext.cpp
    EglGlobalInfo.cpp
    EglValidate.cpp
    EglSurface.cpp
    EglWindowSurface.cpp
    EglPbufferSurface.cpp
    EglThreadInfo.cpp
    EglDisplay.cpp
    EglOsApi_glx.cpp
    ClientAPIExts.cpp)

include_directories(BEFORE
    ${EGL_INCLUDE_DIRS}
    ${GLESv2_INCLUDE_DIRS})

add_library(EGL_translator SHARED ${SOURCES})
target_link_libraries(EGL_translator
    GLcommon
    ${EGL_LDFLAGS}
    ${EGL_LIBRARIES}
    ${GLESv2_LDFLAGS}
    ${GLESv2_LIBRARIES})

install(
  TARGETS EGL_translator
  LIBRARY DESTINATION ${ANBOX_TRANSLATOR_INSTALL_DIR})
