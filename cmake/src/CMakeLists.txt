cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 11)

project(http-server VERSION 1.0 LANGUAGES C)

add_executable(http-server main.c)

include_directories(${CMAKE_SOURCE_DIR}/../lib)

target_include_directories(http-server PUBLIC ${CMAKE_SOURCE_DIR}/../lib)

target_link_libraries(http-server /home/benggee/app/c-program/cmake/lib/mylib/libmylib.so)
target_link_libraries(http-server pthread)