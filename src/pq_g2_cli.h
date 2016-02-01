#ifndef _PQ_G2_CLI_HEADER
#define _PQ_G2_CLI_HEADER

#define PQ_G2_CLI_EXIT_RETCODE 1
#define PQ_G2_PRINTOPTIONS_NOVERBOSE 1

#include "ttd.h"

struct {
    int verbose; // -v
    int normalize; // -N
    uint64_t int_time; // -t

    int16_t channel1; // -1
    int16_t channel2; // -2

    int infile_allocated;
    char *infile; // -o

    int outfile_allocated;
    char *outfile; // -o

    ttd_t bin_time; // -b
    ttd_t window_time; // -w
    int64_t channel2_offset; // -T (note signed time for negative shifts)

    int block_size; // -B
    int rb_size; // -R

} pq_g2_cli_args;

void pq_g2_cli_print_help(char* program_name);

void pq_g2_print_options(int no_verbose);

int pq_g2_read_cli(int argc, char* argv[]);

void pq_g2_cli_cleanup();


#endif
