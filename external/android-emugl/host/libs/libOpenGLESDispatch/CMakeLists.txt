PREPEND(SOURCES ${CMAKE_CURRENT_LIST_DIR}/
    EGLDispatch.cpp
    GLESv2Dispatch.cpp
    GLESv1Dispatch.cpp)

add_library(OpenGLESDispatch STATIC ${SOURCES} ${GLHEADERS_GENERATED_SOURCES})
target_link_libraries(OpenGLESDispatch
    emugl_common
    GLESv2_dec
    GLESv1_dec)
