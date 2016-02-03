add_subdirectory(ttd)
add_subdirectory(pq-parse)
add_subdirectory(pq-filebuffer)
add_subdirectory(pq-ttd)
add_subdirectory(ttd-ringbuffer)

add_executable(ttd-dump ttd_dump.c)
target_link_libraries(ttd-dump LINK_PUBLIC ttd)

add_executable(pq-dump pq_dump.c)
target_link_libraries(pq-dump LINK_PUBLIC ttd-ringbuffer pq-filebuffer pq-ttd-lib pq-parse ttd)

add_executable(ttd-merge ttd_merge.c ttd_filebuffer.c)
target_link_libraries(ttd-merge LINK_PUBLIC ttd)

add_executable(ttd-g2 ttd_g2.c ttd_g2_cli.c ttd_crosscorr2.c ttd_filebuffer.c ttd_g2_cli.c scitoll.c)
target_link_libraries(ttd-g2 LINK_PUBLIC ttd-ringbuffer pq-ttd-lib pq-filebuffer pq-parse ttd )

add_executable(pq-g2 pq_g2.c pq_g2_cli.c ttd_crosscorr2.c ttd_filebuffer.c ttd_g2_cli.c scitoll.c)
target_link_libraries(pq-g2 LINK_PUBLIC ttd-ringbuffer pq-ttd-lib pq-filebuffer ttd pq-parse)

add_executable(ttd-g3 ttd_g3.c ttd_g3_cli.c ttd_crosscorr3.c ttd_filebuffer.c ttd_g2_cli.c scitoll.c)
target_link_libraries(ttd-g3 LINK_PUBLIC ttd-ringbuffer ttd)

add_executable(ttd-g4 ttd_g4.c ttd_g4_cli.c ttd_crosscorr4.c ttd_filebuffer.c ttd_g2_cli.c scitoll.c)
target_link_libraries(ttd-g4 LINK_PUBLIC ttd-ringbuffer ttd)

add_executable(ttd-shift ttd_shift.c scitoll.c ttd_filebuffer.c)
target_link_libraries(ttd-shift LINK_PUBLIC ttd)

install (TARGETS ttd-dump pq-dump ttd-merge ttd-g2 pq-g2 ttd-shift DESTINATION bin)