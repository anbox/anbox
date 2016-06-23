set(ANBOX_SUPPORT_SOURCES
    jni/anbox_support.cpp)

include_directories(
    ${CMAKE_SOURCE_DIR}/external/jni)

# We're not going to ship this, just building here to add proper
# unit tests we can run in a integration system without having
# the Android build system around.
add_library(anbox-support ${ANBOX_SUPPORT_SOURCES})
