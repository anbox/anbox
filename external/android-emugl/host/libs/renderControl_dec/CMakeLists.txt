set(CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/GLESv1_dec)

PREPEND(GENERATED_SOURCES ${CURRENT_BINARY_DIR}/
    renderControl_dec.cpp
    renderControl_server_context.cpp)

add_custom_command(
    OUTPUT ${GENERATED_SOURCES}
    POST_BUILD
    COMMAND mkdir -p ${CURRENT_BINARY_DIR} && ${CMAKE_BINARY_DIR}/external/android-emugl/host/tools/emugen/emugen
            -D ${CURRENT_BINARY_DIR} renderControl
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    DEPENDS emugen)

if ("${cmake_build_type_lower}" STREQUAL "trace")
    set(OPENGL_DEBUG "-DOPENGL_DEBUG_PRINTOUT -DCHECK_GL_ERROR")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPENGL_DEBUG}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPENGL_DEBUG}")
endif()

add_library(renderControl_dec STATIC ${GENERATED_SOURCES})
target_link_libraries(renderControl_dec OpenglCodecCommon)
