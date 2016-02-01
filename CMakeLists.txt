cmake_minimum_required(VERSION 3.3)

project(picoquant)

subdirs(src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")