set(SOURCES
    EGLDispatch.cpp
    GLESv2Dispatch.cpp
    GLESv1Dispatch.cpp)

add_library(OpenGLESDispatch STATIC ${SOURCES})
add_dependencies(OpenGLESDispatch GLHeaders)
target_link_libraries(OpenGLESDispatch
    emugl_common
    GLESv2_dec
    GLESv1_dec)
