set(SOURCES
    shared_state.cpp)

include_directories(BEFORE
    ${MIRCLIENT_INCLUDE_DIRS})

add_library(mir_support ${SOURCES})
target_link_libraries(mir_support
    ${MIRCLIENT_LDFLAGS}
    ${MIRCLIENT_LIBRARIES})
