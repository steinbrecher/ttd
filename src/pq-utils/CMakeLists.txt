### Executables for working with picoquant files ###
# Print records in order to stdout -- mostly used for testing / as demo of pq-filebuffer
add_executable(pq-dump pq_dump.c)
target_link_libraries(pq-dump LINK_PUBLIC pq-filebuffer)

# Compute g2 cross-correlation of photon counts directly from the picoquant file
add_executable(pq-gn pq_gn.c pq_gn_cli.c)
target_link_libraries(pq-gn LINK_PUBLIC pq-filebuffer ttd-crosscorr2 ttd-crosscorr3 ttd-crosscorr4 ttd-filebuffer sci_to_int64 terminal_colors)

install (TARGETS pq-dump pq-gn DESTINATION bin)