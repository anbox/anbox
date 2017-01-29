if (NOT "${HOST_CMAKE_CXX_COMPILER}" STREQUAL "")
    set (CMAKE_CXX_COMPILER "${HOST_CMAKE_CXX_COMPILER}")
endif()

set(SOURCES
    ApiGen.cpp
    EntryPoint.cpp
    main.cpp
    Parser.cpp
    strUtils.cpp
    TypeFactory.cpp)

add_executable(emugen ${SOURCES})
