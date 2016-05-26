# Don't treat any warnings as error as we take the source directly from
# upstream and just compile it.
set(CMAKE_C_FLAGS "-Wall")

include_directories(
    ${CMAKE_SOURCE_DIR}/external/android-emugl/shared
    ${CMAKE_SOURCE_DIR}/external/android-emugl/host/include
    ${CMAKE_SOURCE_DIR}/external/android-emugl/shared/OpenglCodecCommon
    ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs
    ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs/Translator/include
    ${CMAKE_SOURCE_DIR}/external/android-emugl/host/include/libOpenglRender
    ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs/GLESv1_dec
    ${CMAKE_BINARY_DIR}/external/android-emugl/host/libs/GLESv1_dec
    ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs/GLESv2_dec
    ${CMAKE_BINARY_DIR}/external/android-emugl/host/libs/GLESv2_dec
    ${CMAKE_SOURCE_DIR}/external/android-emugl/host/libs/renderControl_dec
    ${CMAKE_BINARY_DIR}/external/android-emugl/host/libs/renderControl_dec)

add_subdirectory(host)
add_subdirectory(shared)
