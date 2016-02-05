cmake_minimum_required(VERSION 3.3)

project(picoquant)

subdirs(src)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -Wall")