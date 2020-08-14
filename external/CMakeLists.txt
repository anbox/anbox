set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error")

add_subdirectory(process-cpp-minimal)
add_subdirectory(android-emugl)
add_subdirectory(xdg)
add_subdirectory(backward-cpp)

include(ExternalProject)

ExternalProject_Add(CpuFeatures
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cpu_features"
	INSTALL_COMMAND ""
	CMAKE_CACHE_ARGS
		-DBUILD_SHARED_LIBS:BOOL=OFF
		-DBUILD_TESTING:BOOL=OFF
)

ExternalProject_Add(sdbus-cpp
	PREFIX sdbus-cpp
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdbus-cpp"
	INSTALL_COMMAND ""
	CMAKE_CACHE_ARGS
		-DBUILD_CODE_GEN:BOOL=ON
		-DBUILD_SHARED_LIBS:BOOL=OFF
)
