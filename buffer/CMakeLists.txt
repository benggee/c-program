cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 11)

project(buffer VERSION 1.0 LANGUAGES C)

add_executable(server server.c buffer.c)

add_executable(client client.c buffer.c)

target_link_libraries(server pthread)
target_link_libraries(client pthread)