set(COMMON_SOURCES
    crash_reporter.cpp
    id_to_object_map.cpp
    lazy_instance.cpp
    logging.cpp
    message_channel.cpp
    pod_vector.cpp
    shared_library.cpp
    smart_ptr.cpp
    sockets.cpp
    thread_store.cpp
    thread_pthread.cpp)

add_library(emugl_common ${COMMON_SOURCES})
target_link_libraries(emugl_common
    dl pthread)
