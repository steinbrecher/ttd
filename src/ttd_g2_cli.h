#ifndef _TTD_G2_CLI_HEADER
#define _TTD_G2_CLI_HEADER

#define TTD_G2_CLI_EXIT_RETCODE 1
#define TTD_G2_PRINTOPTIONS_NOVERBOSE 1

#include "ttd.h"

struct {
  int verbose; // -v

  int infiles_allocated[2];
  char *infile1; // -1
  char *infile2; // -2

  int outfile_allocated;
  char *outfile; // -o


  ttd_t bin_time; // -b
  ttd_t window_time; // -w
  int64_t infile2_offset; // -T (note signed time for negative shifts)

  int block_size; // -B

} ttd_g2_cli_args;

void ttd_g2_cli_print_help(char* program_name);

void ttd_g2_print_options();

int ttd_g2_read_cli(int argc, char* argv[]);

void ttd_g2_cli_cleanup();


#endif
