# Add library for functionality
add_library(pq-ttd-lib pq_ttd.c)
target_link_libraries(pq-ttd-lib LINK_PUBLIC pq-parse ttd)
target_include_directories(pq-ttd-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add executable for direct access
add_executable(pq-ttd pq_ttd_main.c pq_ttd_cli.c)
target_link_libraries(pq-ttd LINK_PUBLIC pq-ttd-lib)
install (TARGETS pq-ttd DESTINATION bin)