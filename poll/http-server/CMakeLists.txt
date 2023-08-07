cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 11)

project(buffer VERSION 1.0 LANGUAGES C)

file(GLOB SOURCES *.c)

add_executable(server ${SOURCES})

target_link_libraries(server pthread)