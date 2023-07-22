cmake_minimum_required(VERSION 3.15)

project(mylib)

file(GLOB SRC_FILES *.c)
add_library(mylib SHARED ${SRC_FILES})