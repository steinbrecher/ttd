# Parses a scientific number from text to long long
add_library(scitoll scitoll.c)
target_include_directories(scitoll PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})