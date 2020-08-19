if (NOT PROJECT_NAME)
    project(emugen)
endif()

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

# Make sure to install emugen into your sysroot
if (${PROJECT_NAME} STREQUAL "emugen")
    install(
      TARGETS emugen
      RUNTIME DESTINATION bin
      LIBRARY DESTINATION lib
    )
endif()