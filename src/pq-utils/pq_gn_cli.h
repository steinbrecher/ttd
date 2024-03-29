#ifndef _PQ_GN_CLI_HEADER
#define _PQ_GN_CLI_HEADER

#define PQ_GN_CLI_EXIT_RETCODE 1
#define PQ_GN_PRINTOPTIONS_NOVERBOSE 1

#define PQ_GN_MAX_CORRELATION_ORDER 4

#define COLORFUL_OUTPUT

#include "colorful_output.h"

#include "ttd.h"
#include "pq_parse.h"

struct {
    int verbose; // -v
    int normalize; // -N
    uint64_t int_time; // -t

    int16_t channel1; // -1
    int16_t channel2; // -2

    int64_t channel_offset[PQ_HH_MAX_CHANNELS];
    _Bool channel_active[PQ_HH_MAX_CHANNELS];

    char *infile; // -o

    char *outfile_prefix; // -o

    // This is padded by 1 to make code more obvious & easier to read
    // If activeCorrelationOrders[3] == 1, will compute G(3)
    _Bool activeCorrelationOrders[PQ_GN_MAX_CORRELATION_ORDER + 1]; // -g

    ttd_t bin_time; // -b
    ttd_t window_time; // -w
    ttd_t chunk_time; // -c
    ttd_t padded_window_time;
    int64_t channel2_offset; // -T (note signed time for negative shifts)

    size_t block_size; // -B
    size_t rb_size; // -R

} pq_gn_cli_args;

void pq_gn_cli_print_help(char *program_name);

void pq_gn_print_options(int no_verbose);

int pq_gn_read_cli(int argc, char **argv);

int check_pq_gn_cli_args();

void pq_gn_cli_cleanup();


#endif
