AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = pq-ttd ttd-g2 ttd-merge ttd-delay ttd-dump

scitollu = src/scitollu.c
ttd = src/ttd.c
ttp = src/ttp.c
ttp_cli = src/ttp_cli.c ${ttd} ${ttp}
ttd_ringbuffer = src/ttd_ringbuffer.c ${ttp_cli}
ttd_crosscorr = src/ttd_crosscorr.c ${ttd_ringbuffer}
ttd_filebuffer = src/ttd_filebuffer.c

pq_ttd_SOURCES = src/pq_ttd.c src/pq_ttd_cli.c src/pq_parse.c src/ttd.c src/ttp.c
ttd_dump_SOURCES = src/ttd_dump.c src/ttd.h
ttd_merge_SOURCES = src/ttd_merge.c src/ttd_filebuffer.c src/ttd.c
ttd_g2_SOURCES = src/ttd_g2.c ${ttd_crosscorr} ${ttd_filebuffer} ${scitollu}
ttd_delay_SOURCES = src/ttd_delay.c ${scitollu} ${ttd_filebuffer} ${ttd} ${ttp}
