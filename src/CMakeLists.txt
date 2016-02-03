add_subdirectory(ttd)
add_subdirectory(pq-parse)
add_subdirectory(pq-filebuffer)
add_subdirectory(pq-ttd)
add_subdirectory(ttd-ringbuffer)

### Executables for working with picoquant files ###
# Print records in order to stdout -- mostly used for testing / as demo of pq-filebuffer
add_executable(pq-dump pq_dump.c)
target_link_libraries(pq-dump LINK_PUBLIC pq-filebuffer)

# Compute g2 cross-correlation of photon counts directly from the picoquant file
add_executable(pq-g2 pq_g2.c pq_g2_cli.c ttd_crosscorr2.c ttd_filebuffer.c ttd_g2_cli.c scitoll.c)
target_link_libraries(pq-g2 LINK_PUBLIC pq-filebuffer)


### Executables for working with ttd files ###
# Print records from ttd file to stdout
add_executable(ttd-dump ttd_dump.c)
target_link_libraries(ttd-dump LINK_PUBLIC ttd)

# Merge two ttd files, maintaining time ordering of records
add_executable(ttd-merge ttd_merge.c ttd_filebuffer.c)
target_link_libraries(ttd-merge LINK_PUBLIC ttd)

# Shift all records in a ttd file forward or backward
add_executable(ttd-shift ttd_shift.c scitoll.c ttd_filebuffer.c)
target_link_libraries(ttd-shift LINK_PUBLIC ttd)

# Compute g2 cross-correlation function between records in two ttd files
add_executable(ttd-g2 ttd_g2.c ttd_g2_cli.c ttd_crosscorr2.c ttd_filebuffer.c ttd_g2_cli.c scitoll.c)
target_link_libraries(ttd-g2 LINK_PUBLIC pq-ttd-lib ttd-ringbuffer)

# Compute g3 cross-correlation function between records in three ttd files
add_executable(ttd-g3 ttd_g3.c ttd_g3_cli.c ttd_crosscorr3.c ttd_filebuffer.c ttd_g2_cli.c scitoll.c)
target_link_libraries(ttd-g3 LINK_PUBLIC ttd-ringbuffer)

# Compute g4 cross-correlation function between records in four ttd files
add_executable(ttd-g4 ttd_g4.c ttd_g4_cli.c ttd_crosscorr4.c ttd_filebuffer.c ttd_g2_cli.c scitoll.c)
target_link_libraries(ttd-g4 LINK_PUBLIC ttd-ringbuffer)

install (TARGETS ttd-dump pq-dump ttd-merge ttd-g2 pq-g2 ttd-shift DESTINATION bin)