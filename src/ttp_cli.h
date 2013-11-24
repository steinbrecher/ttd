#ifndef _TTP_CLI_HEADER
#define _TTP_CLI_HEADER

#define TTP_CLI_EXIT_RETCODE 1
#define TTP_PRINTOPTIONS_NOVERBOSE 1

struct {
  int verbose; // -v

  int infiles_allocated[2];
  char *infile1; // -i
  char *infile2; // -I

  int outfiles_allocated[2];
  char *outfile1; // -o
  char *outfile2; // -O

  uint64_t bin_time; // -b
  uint64_t window_time; // -w

  int block_size; // -B

} ttp_cli_args;

void ttp_cli_print_help(char* program_name);

void ttp_print_options();

int ttp_read_cli(int argc, char* argv[]);

void ttp_cli_cleanup();


#endif
