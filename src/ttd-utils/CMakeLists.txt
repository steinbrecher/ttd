### General Utilities ###
# Print records from ttd file to stdout
add_executable(ttd-dump ttd_dump.c)
target_link_libraries(ttd-dump LINK_PUBLIC ttd)

# Merge two ttd files, maintaining time ordering of records
add_executable(ttd-merge ttd_merge.c)
target_link_libraries(ttd-merge LINK_PUBLIC ttd-filebuffer)

# Shift all records in a ttd file forward or backward
add_executable(ttd-shift ttd_shift.c)
target_link_libraries(ttd-shift LINK_PUBLIC ttd-filebuffer sci_to_int64)

# Decimate the records in a ttd file
add_executable(ttd-decimate ttd_decimate.c)
target_link_libraries(ttd-decimate LINK_PUBLIC ttd-filebuffer)

### Autocorrelation Executables ###
# Add executable for direct access
# Compute g2 cross-correlation function between records in two ttd files
add_executable(ttd-g2 ttd_g2.c ttd_g2_cli.c ttd_g2_cli.c)
target_link_libraries(ttd-g2 LINK_PUBLIC pq-ttd-lib ttd-ringbuffer ttd-crosscorr2 ttd-filebuffer sci_to_int64)

# Compute g3 cross-correlation function between records in three ttd files
add_executable(ttd-g3 ttd_g3.c ttd_g3_cli.c ttd_g2_cli.c)
target_link_libraries(ttd-g3 LINK_PUBLIC ttd-ringbuffer ttd-crosscorr3 ttd-filebuffer sci_to_int64)

# Compute g4 cross-correlation function between records in four ttd files
add_executable(ttd-g4 ttd_g4.c ttd_g4_cli.c ttd_g2_cli.c)
target_link_libraries(ttd-g4 LINK_PUBLIC ttd-ringbuffer ttd-crosscorr4 ttd-filebuffer sci_to_int64)

install (TARGETS ttd-g2 ttd-g3 ttd-g4 ttd-dump ttd-merge ttd-shift ttd-decimate DESTINATION bin)