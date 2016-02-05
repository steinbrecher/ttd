### ttd-filebuffer ###
# Handles parsing, buffering, and potentially shifting .ttd files
add_library(ttd-filebuffer ttd_filebuffer.c)
target_link_libraries(ttd-filebuffer LINK_PUBLIC ttd)
target_include_directories(ttd-filebuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

### ttd-ringbuffer ###
# Provides ring buffer (also called circular buffer) for ttd records
add_library(ttd-ringbuffer ttd_ringbuffer.c)
target_link_libraries(ttd-ringbuffer LINK_PUBLIC ttd)
target_include_directories(ttd-ringbuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

### ttd_crosscorr2/3/4 ###
# Second-, third-, and fourth-order correlation trackers for ttd records
add_library(ttd-crosscorr2 ttd_crosscorr2.c)
add_library(ttd-crosscorr3 ttd_crosscorr3.c)
add_library(ttd-crosscorr4 ttd_crosscorr4.c)

target_link_libraries(ttd-crosscorr2 LINK_PUBLIC ttd ttd-ringbuffer)
target_link_libraries(ttd-crosscorr3 LINK_PUBLIC ttd ttd-ringbuffer)
target_link_libraries(ttd-crosscorr4 LINK_PUBLIC ttd ttd-ringbuffer)

target_include_directories(ttd-crosscorr2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ttd-crosscorr3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(ttd-crosscorr4 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

