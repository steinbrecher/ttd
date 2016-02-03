add_library(ttd-ringbuffer ttd_ringbuffer.c)
target_link_libraries(ttd-ringbuffer LINK_PUBLIC ttd)
target_include_directories(ttd-ringbuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})