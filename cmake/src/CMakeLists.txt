cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 11)

project(http-server VERSION 1.0 LANGUAGES C)


include_directories(${CMAKE_SOURCE_DIR}/../include)

add_subdirectory(${CMAKE_SOURCE_DIR}/../lib ${CMAKE_SOURCE_DIR}/../lib/build)


add_executable(http-server main.c)

include_directories(${CMAKE_SOURCE_DIR}/../lib)

target_include_directories(http-server PUBLIC ${CMAKE_SOURCE_DIR}/../lib)

target_link_libraries(http-server PRIVATE ${CMAKE_SOURCE_DIR}/../build/lib/libmylib.so)

install(TARGETS http-server DESTINATION ${CMAKE_SOURCE_DIR}/../build/bin)
install(FILES ${CMAKE_SOURCE_DIR}/../lib/build/config.h DESTINATION ${CMAKE_SOURCE_DIR}/../include)
install(FILES ${CMAKE_SOURCE_DIR}/../lib/build/libmylib.so DESTINATION ${CMAKE_SOURCE_DIR}/../build/lib)