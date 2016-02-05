# Generally useful functions / libraries that aren't pq/ttd specific
add_subdirectory(gen-libs)

# General library definining the ttd datatype. Used throughout, but should probably be deprecated
add_subdirectory(ttd)

# Libraries to support working with picoquant files from C
add_subdirectory(pq-lib)

# Utilities (i.e. executables to be built) for working with picoquant files
add_subdirectory(pq-utils)

# Libraries to support working with ttd files from C
add_subdirectory(ttd-lib)

# Utilities (i.e. executables to be built) for working with .ttd files
add_subdirectory(ttd-utils)

# Conversion from picoquant to ttd data format
# TODO: Reverse conversion (not a pressing issue -- i.e. "someday")
add_subdirectory(pq-ttd)