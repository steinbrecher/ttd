add_library(ttd ttd.c)
target_include_directories(ttd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ttd LINK_PUBLIC m)