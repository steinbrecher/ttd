add_library(ttd-filebuffer ttd_filebuffer.c)
target_link_libraries(ttd-filebuffer LINK_PUBLIC ttd)
target_include_directories(ttd-filebuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})