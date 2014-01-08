AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = pq-ttd ttd-g2 ttd-g3 ttd-g4 ttd-merge ttd-delay ttd-dump

scitoll = src/scitoll.c
ttd = src/ttd.c
ttd_ringbuffer = src/ttd_ringbuffer.c
ttd_crosscorr2 = src/ttd_crosscorr2.c ${ttd_ringbuffer}
ttd_crosscorr3 = src/ttd_crosscorr3.c ${ttd_ringbuffer}
ttd_crosscorr4 = src/ttd_crosscorr4.c ${ttd_ringbuffer}
ttd_filebuffer = src/ttd_filebuffer.c ${ttd}

pq_ttd_SOURCES = src/pq_ttd.c src/pq_ttd_cli.c src/pq_parse.c src/ttd.c 
ttd_dump_SOURCES = src/ttd_dump.c src/ttd.h
ttd_merge_SOURCES = src/ttd_merge.c src/ttd_filebuffer.c src/ttd.c
ttd_g2_SOURCES = src/ttd_g2.c src/ttd_g2_cli.c ${ttd_crosscorr2} ${ttd_filebuffer} ${ttd_g2_cli} ${scitoll}
ttd_g3_SOURCES = src/ttd_g3.c src/g3_cli.c ${ttd_crosscorr3} ${ttd_filebuffer} ${g3_cli} ${scitoll}
ttd_g4_SOURCES = src/ttd_g4.c src/ttd_g4_cli.c ${ttd_crosscorr4} ${ttd_filebuffer} ${scitoll}
ttd_delay_SOURCES = src/ttd_delay.c ${scitoll} ${ttd_filebuffer}
