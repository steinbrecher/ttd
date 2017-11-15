#ifndef _G3_CLI_HEADER
#define _G3_CLI_HEADER

#define G3_CLI_EXIT_RETCODE 1
#define G3_CLI_PRINTOPTIONS_NOVERBOSE 1

#include "ttd.h"

struct {
    _Bool verbose; // -v

    char *infile1; // -1
    char *infile2; // -2
    char *infile3; // -3

    char *outfile; // -o

    ttd_t bin_time; // -b
    ttd_t window_time; // -w

    size_t block_size; // -B

} g3_cli_args;

void g3_cli_print_help(char *program_name);

void g3_cli_print_options(_Bool no_verbose);

int g3_read_cli(int argc, char *argv[]);

void g3_cli_cleanup();


#endif // _G3_CLI_HEADER
