add_library(pq-filebuffer pq_filebuffer.c)
target_link_libraries(pq-filebuffer LINK_PUBLIC ttd-ringbuffer pq-ttd-lib pq-parse ttd)
target_include_directories(pq-filebuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(pq-parse pq_parse.c)
target_link_libraries(pq-parse LINK_PUBLIC ttd colorful_output)
target_include_directories(pq-parse PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})