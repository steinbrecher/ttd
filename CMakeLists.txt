cmake_minimum_required(VERSION 3.3)



project(picoquant)

subdirs(src)

set(CMAKE_BUILD_TYPE DEBUG)
#set(CMAKE_BUILD_TYPE RELEASE)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O2 -g -Wall -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O2 -g -Wall -fsanitize=undefined")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -flto")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -flto")


# Local debugging compiler, to allow for -fsanitize
if(CMAKE_BUILD_TYPE EQUAL DEBUG)
    set(CMAKE_C_COMPILER "/opt/local/bin/clang-mp-3.9")
    set(CMAKE_CXX_COMPILER "/opt/local/bin/clang++-mp-3.9")
endif(CMAKE_BUILD_TYPE EQUAL DEBUG)