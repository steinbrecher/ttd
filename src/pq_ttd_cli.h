#ifndef _PQ_TTD_CLI_HEADER
#define _PQ_TTD_CLI_HEADER

#define PQ_TTD_CLI_EXIT_RETCODE 1
#define PQ_TTD_PRINTOPTIONS_NOVERBOSE 1

#include "ttd.h"
#include "pq_ttd.h"

struct {
  int verbose; // -v
  int compress; // -c

  int infile_allocated;
  char *infile; // -i

  int output_prefix_allocated;
  char *output_prefix; // -o

  int block_size;

} pq_ttd_cli_args;

void pq_ttd_cli_print_help(char* program_name);

void pq_ttd_print_options();

int pq_ttd_read_cli(int argc, char* argv[]);

void pq_ttd_cli_cleanup();


#endif
