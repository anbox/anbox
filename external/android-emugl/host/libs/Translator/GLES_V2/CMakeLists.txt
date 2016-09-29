set(SOURCES
    GLESv2Imp.cpp
    GLESv2Context.cpp
    GLESv2Validate.cpp
    ShaderParser.cpp
    ProgramData.cpp)

add_library(GLES_V2_translator SHARED ${SOURCES})
target_link_libraries(GLES_V2_translator GLcommon)

install(
  TARGETS GLES_V2_translator
  LIBRARY DESTINATION ${ANBOX_TRANSLATOR_INSTALL_DIR})
