add_library(pq-parse pq_parse.c)
target_link_libraries(pq-parse LINK_PUBLIC ttd)
target_include_directories(pq-parse PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})