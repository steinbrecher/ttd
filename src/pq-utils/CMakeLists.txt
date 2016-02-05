### Executables for working with picoquant files ###
# Print records in order to stdout -- mostly used for testing / as demo of pq-filebuffer
add_executable(pq-dump pq_dump.c)
target_link_libraries(pq-dump LINK_PUBLIC pq-filebuffer)

# Compute g2 cross-correlation of photon counts directly from the picoquant file
add_executable(pq-g2 pq_g2.c pq_g2_cli.c)
target_link_libraries(pq-g2 LINK_PUBLIC pq-filebuffer ttd-crosscorr2 ttd-filebuffer scitoll)

install (TARGETS pq-dump pq-g2 DESTINATION bin)