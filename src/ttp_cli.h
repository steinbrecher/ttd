#ifndef _TTP_CLI_HEADER
#define _TTP_CLI_HEADER

#define TTP_CLI_EXIT_RETCODE 1

void ttp_cli_print_help(char* program_name);
void ttp_print_options();
int ttp_read_cli(int argc, char* argv[]);

struct {
  int verbose; // -v

  char *infile1; // -i
  char *infile2; // -I

  char *outfile1; // -o
  char *outfile2; // -O

  uint64_t bin_time; // -b
  uint64_t window_time; // -w

} ttp_cli_args;

#endif
