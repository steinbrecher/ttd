AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = pq-ttd ttd-g2 ttd-g3 ttd-merge ttd-delay ttd-dump

scitollu = src/scitollu.c
ttd = src/ttd.c
ttp = src/ttp.c
ttd_g2_cli = src/ttd_g2_cli.c ${ttd} ${ttp}
g3_cli = src/g3_cli.c ${ttd} ${ttp}
ttd_ringbuffer = src/ttd_ringbuffer.c
ttd_crosscorr2 = src/ttd_crosscorr2.c ${ttd_ringbuffer}
ttd_crosscorr3 = src/ttd_crosscorr3.c ${ttd_ringbuffer}
ttd_filebuffer = src/ttd_filebuffer.c

pq_ttd_SOURCES = src/pq_ttd.c src/pq_ttd_cli.c src/pq_parse.c src/ttd.c src/ttp.c
ttd_dump_SOURCES = src/ttd_dump.c src/ttd.h
ttd_merge_SOURCES = src/ttd_merge.c src/ttd_filebuffer.c src/ttd.c
ttd_g2_SOURCES = src/ttd_g2.c ${ttd_crosscorr2} ${ttd_filebuffer} ${ttd_g2_cli} ${scitollu}
ttd_g3_SOURCES = src/ttd_g3.c ${ttd_crosscorr3} ${ttd_filebuffer} ${g3_cli} ${scitollu}
ttd_delay_SOURCES = src/ttd_delay.c ${scitollu} ${ttd_filebuffer} ${ttd} ${ttp}
