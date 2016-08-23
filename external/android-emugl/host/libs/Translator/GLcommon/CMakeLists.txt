set(SOURCES
    GLDispatch.cpp
    GLutils.cpp
    GLEScontext.cpp
    GLESvalidate.cpp
    GLESpointer.cpp
    GLESbuffer.cpp
    RangeManip.cpp
    TextureUtils.cpp
    PaletteTexture.cpp
    etc1.cpp
    objectNameManager.cpp
    FramebufferData.cpp)

add_library(GLcommon ${SOURCES})
target_link_libraries(GLcommon
    emugl_common)
