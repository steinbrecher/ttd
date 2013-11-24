#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include "ttp.h"
#include "ttp_cli.h"

// Common command line options defined here to unify interface across different programs

static const struct option ttp_longopts[] = {
  { "version", no_argument, NULL, 'v' },
  { "help", no_argument, NULL, 'h' },

  { "verbose", no_argument, NULL, 'V' },

  { "input-file-1", required_argument, NULL, 'i' },
  { "output-file-2", required_argument, NULL, 'I' },

  { "output-file-1", required_argument, NULL, 'o' },
  { "output-file-2", required_argument, NULL, 'O' },

  { "bin-time", required_argument, NULL, 'b' },
  { "window-time", required_argument, NULL, 'w' },
};

static const char *ttp_optstring = "Vhvi:I:o:O:b:w:";

void ttp_cli_print_help(char* program_name) {
  // Need a string of spaces equal in length to the program name
  printf("Usage: %s [options]\n", program_name);
  printf("Example: %s -b 1000 -w 100000 -i channel1.ttd -I channel2.ttd -o results.csv\n", program_name);
  printf("Options without parameters:\n");
  printf("    -h / --help:    Print help dialog\n");
  printf("    -v / --version: Print program version\n");
  printf("    -V / --verbose: Print program version\n");
  printf("Options with parameters:\n");
  printf("    -i / --input-file-1 [file]\n");
  printf("    -I / --input-file-2 [file]\n");
  printf("    -o / --output-file-1 [file]\n");
  printf("    -O / --output-file-2 [file]\n");
  printf("    -b / --bin-time [time in ps]\n");
  printf("    -w / --window-time [time in ps]\n");
}

int ttp_read_cli(int argc, char* argv[]) {
  // Initialize default values
  ttp_cli_args.verbose = 0;

  ttp_cli_args.bin_time = 0;
  ttp_cli_args.window_time = 0;

  // Read command line options
  int option_index, opt;
  opt = getopt_long(argc, argv, ttp_optstring, ttp_longopts, &option_index);
  while (opt != -1) {
    switch (opt) {
    case 'v':
      ttp_cli_args.verbose = 1;
      break;

    case 'V':
      ttp_print_version(argv[0]);
      exit(TTP_CLI_EXIT_RETCODE);
      break;

    case 'h':
      ttp_cli_print_help(argv[0]);
      exit(TTP_CLI_EXIT_RETCODE);
      break;

    case 'i':
      ttp_cli_args.infile1 = (char *)malloc(sizeof(optarg));
      strcpy(ttp_cli_args.infile1, optarg);
      break;
    case 'I':
      ttp_cli_args.infile2 = (char *)malloc(sizeof(optarg));
      strcpy(ttp_cli_args.infile2, optarg);
      break;

    case 'o':
      ttp_cli_args.outfile1 = (char *)malloc(sizeof(optarg));
      strcpy(ttp_cli_args.outfile1, optarg);
      break;
    case 'O':
      ttp_cli_args.outfile2 = (char *)malloc(sizeof(optarg));
      strcpy(ttp_cli_args.outfile2, optarg);
      break;
      
    case 'b':
      ttp_cli_args.bin_time = atoi(optarg);
      break;
    case 'w':
      ttp_cli_args.window_time = atoi(optarg);
      break;

    default:
      // Shouldn't actually get here
      break;
    }
    opt = getopt_long(argc, argv, ttp_optstring, ttp_longopts, &option_index);
  }
  return(0);
}

void ttp_print_options() {
  printf("Verbose: %d\n", ttp_cli_args.verbose);
  if (ttp_cli_args.infile1 != NULL) {
    printf("Infile 1: %s\n", ttp_cli_args.infile1);
  }
  if (ttp_cli_args.infile2 != NULL) {
    printf("Infile 2: %s\n", ttp_cli_args.infile2);
  }
  if (ttp_cli_args.outfile1 != NULL) {
    printf("Outfile 1: %s\n", ttp_cli_args.outfile1);
  }
  if (ttp_cli_args.outfile2 != NULL) {
    printf("Outfile 2: %s\n", ttp_cli_args.outfile2);
  }
  printf("Bin Time: %" PRIu64 "\n", ttp_cli_args.bin_time);
  printf("Window Time: %" PRIu64 "\n", ttp_cli_args.window_time);
}  
