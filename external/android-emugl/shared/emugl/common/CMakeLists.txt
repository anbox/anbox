set(COMMON_SOURCES
    condition_variable.h
    crash_reporter.cpp
    crash_reporter.h
    id_to_object_map.cpp
    id_to_object_map.h
    lazy_instance.cpp
    lazy_instance.h
    logging.cpp
    logging.h
    message_channel.cpp
    message_channel.h
    mutex.h
    mutex_unittest.cpp
    pod_vector.cpp
    pod_vector.h
    scoped_pointer_vector.h
    shared_library.cpp
    shared_library.h
    smart_ptr.cpp
    smart_ptr.h
    sockets.cpp
    sockets.h
    thread.h
    thread_pthread.cpp
    thread_store.cpp
    thread_store.h
    thread_unittest.cpp
    unique_integer_map.h)

add_library(emugl_common STATIC ${COMMON_SOURCES})
target_link_libraries(emugl_common
    dl pthread)
