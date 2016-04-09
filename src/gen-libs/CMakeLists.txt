# Parses a scientific number from text to long long
add_library(sci_to_int64 sci_to_int64.c)
add_library(terminal_colors terminal_colors.c)
target_include_directories(sci_to_int64 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(terminal_colors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})