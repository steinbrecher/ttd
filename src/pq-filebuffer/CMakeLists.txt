add_library(pq-filebuffer pq_filebuffer.c)
target_link_libraries(pq-filebuffer LINK_PUBLIC ttd-ringbuffer pq-ttd-lib pq-parse ttd)
target_include_directories(pq-filebuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})