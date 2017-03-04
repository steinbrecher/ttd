# Parses a scientific number from text to long long
add_library(sci_to_int64 sci_to_int64.c)
add_library(colorful_output colorful_output.c)
target_include_directories(sci_to_int64 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(colorful_output PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})