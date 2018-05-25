set(GENERATED_SOURCES
    gles1_extensions_functions.h
    gles1_only_functions.h
    gles2_extensions_functions.h
    gles2_only_functions.h
    gles3_only_functions.h
    gles_common_functions.h
    gles_extensions_functions.h
    gles_functions.h
    RenderEGL_extensions_functions.h
    RenderEGL_functions.h)

add_custom_target(GLHeaders)
add_custom_command(
    TARGET GLHeaders
    POST_BUILD
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/gen-emugl-headers.sh ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_subdirectory(GLESv1_dec)
add_subdirectory(GLESv2_dec)
add_subdirectory(libOpenGLESDispatch)
add_subdirectory(renderControl_dec)
