#ifndef _TTD_G4_CLI_HEADER
#define _TTD_G4_CLI_HEADER

#define G4_CLI_EXIT_RETCODE 1
#define G4_CLI_PRINTOPTIONS_NOVERBOSE 1

#include "ttd.h"

struct {
  _Bool verbose; // -v

  char *infiles[4]; // -1 thru -4

  char *outfile; // -o

  ttd_t bin_time; // -b
  ttd_t window_time; // -w

  size_t block_size; // -B

} g4_cli_args;

void g4_cli_print_help(char* program_name);

void g4_cli_print_options(int no_verbose);

int g4_read_cli(int argc, char* argv[]);

void g4_cli_cleanup();


#endif // _G4_CLI_HEADER
