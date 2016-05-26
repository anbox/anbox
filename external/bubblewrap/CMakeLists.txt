# Don't treat any warnings as error as we take the source directly from
# upstream and just compile it.
set(CMAKE_C_FLAGS "-Wall")

set(SOURCES
    bind-mount.c
    bubblewrap.c
    network.c
    utils.c)

set(HEADERS
    config.h)

add_library(bwrap ${SOURCES} ${HEADERS})
