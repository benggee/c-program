file(GLOB SRC_FILES *.c)
add_library(mylib SHARED ${SRC_FILES})
target_link_libraries(mylib pthread)

if (${POOL_SIZE})
    set(WORKER_POOL_SIZE ${POOL_SIZE})
endif ()

configure_file(config.h.cmake config.h)