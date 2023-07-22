file(GLOB SRC_FILES *.c)
add_library(mylib SHARED ${SRC_FILES})
target_link_libraries(mylib pthread)