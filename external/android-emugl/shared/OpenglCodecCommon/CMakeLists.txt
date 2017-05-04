set(SOURCES
    ChecksumCalculator.cpp
    ChecksumCalculator.h
    ChecksumCalculatorThreadInfo.cpp
    ChecksumCalculatorThreadInfo.h
    CMakeLists.txt
    ErrorLog.h
    gl_base_types.h
    GLDecoderContextData.h
    glUtils.cpp
    glUtils.h
    Makefile
    ProtocolUtils.h)

add_library(OpenglCodecCommon STATIC ${SOURCES})
