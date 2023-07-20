cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 11)

project(http-server VERSION 1.0 LANGUAGES C)

include_directories(..)

add_executable(http-server main.c
        http_response.c
        reader.c
        tcp_server.c
        thread_pool.c)

target_link_libraries(http-server pthread)