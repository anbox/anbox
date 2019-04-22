FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

PREPEND(GLHEADERS_SOURCES libOpenGLESDispatch/
    render_egl_extensions.entries
    render_egl.entries
    gles_common.entries
    gles_extensions.entries
    gles1_only.entries
    gles1_extensions.entries
    gles2_only.entries
    gles2_extensions.entries
    gles3_only.entries)

PREPEND(GLHEADERS_GENERATED_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/../include/OpenGLESDispatch/
    gles1_extensions_functions.h
    gles1_only_functions.h
    gles2_extensions_functions.h
    gles2_only_functions.h
    gles3_only_functions.h
    gles_common_functions.h
    gles_extensions_functions.h
    RenderEGL_extensions_functions.h
    RenderEGL_functions.h)

add_custom_command(
    OUTPUT ${GLHEADERS_GENERATED_SOURCES}
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/gen-emugl-headers.sh ${CMAKE_BINARY_DIR}
    DEPENDS ${GLHEADERS_SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

include(GLESv1_dec/CMakeLists.txt)
include(GLESv2_dec/CMakeLists.txt)
include(libOpenGLESDispatch/CMakeLists.txt)
include(renderControl_dec/CMakeLists.txt)
