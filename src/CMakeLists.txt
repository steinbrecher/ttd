#add_executable(pq-ttd pq_ttd.c pq_ttd_cli.c pq_parse.c ttd.c)
add_executable(ttd-dump ttd_dump.c)
add_executable(pq-dump pq_dump.c pq_filebuffer.c pq_ttd.c ttd.c pq_parse.c)
add_executable(ttd-merge ttd_merge.c ttd_filebuffer.c ttd.c)

add_executable(ttd-g2 ttd_g2.c pq_filebuffer.c pq_ttd.c pq_ttd_cli.c pq_parse.c ttd_g2_cli.c ttd_crosscorr2.c ttd_ringbuffer.c ttd_filebuffer.c ttd_g2_cli.c scitoll.c ttd.c)
add_executable(pq-g2 pq_g2.c ttd_g2.c pq_filebuffer.c pq_ttd.c pq_ttd_cli.c pq_parse.c ttd_g2_cli.c ttd_crosscorr2.c ttd_ringbuffer.c ttd_filebuffer.c ttd_g2_cli.c scitoll.c ttd.c)
add_executable(ttd-g3 ttd_g3.c ttd_g3_cli.c ttd_crosscorr3.c ttd_ringbuffer.c ttd_filebuffer.c ttd_g2_cli.c scitoll.c ttd.c)
add_executable(ttd-g4 ttd_g4.c ttd_g4_cli.c ttd_crosscorr4.c ttd_ringbuffer.c ttd_filebuffer.c ttd_g2_cli.c scitoll.c ttd.c)
add_executable(ttd-shift ttd_shift.c scitoll.c ttd_filebuffer.c ttd.c)

install (TARGETS ttd-dump pq-dump ttd-merge ttd-g2 pq-g2 ttd-shift DESTINATION bin)