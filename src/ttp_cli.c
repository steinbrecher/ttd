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

  { "block-size", required_argument, NULL, 'B' },
};

static const char *ttp_optstring = "Vhvi:I:o:O:b:w:B:";

void ttp_cli_print_help(char* program_name) {
  // Need a string of spaces equal in length to the program name
  int len = strlen(program_name)+1;
  char pn_spaces[len];
  int i;
  for (i=0; i<len-1; i++) {
      pn_spaces[i] = ' ';
    }
  pn_spaces[len-1] = '\0';
  printf("Usage: %s [-i input-file-1] [-I input-file-2] [-o output-file-1]\n", program_name);
  printf("       %s [-O output-file-2] [-b bin-time] [-w window-time]\n", pn_spaces);
  printf("       %s [-B block-size]\n", pn_spaces);

  printf("\tNotes: \n");
  printf("\t\t-b (--bin-time):    Specified in picoseconds\n");
  printf("\t\t-w (--window-time): Window time in picoseconds\n");
  printf("\t\t-B (--block-size):  Number of photon records to read into RAM at a time.\n");
  printf("\t\t                    Experimental option; don't change unless you have a good reason.\n");

  printf("\tOther options:\n");
  printf("\t\t-h / --help:    Print help dialog\n");
  printf("\t\t-v / --version: Print program version\n");
  printf("\t\t-V / --verbose: Print program version\n");
}

int ttp_read_cli(int argc, char* argv[]) {
  // Initialize default values
  ttp_cli_args.verbose = 0;

  ttp_cli_args.bin_time = 10;
  ttp_cli_args.window_time = 10000;

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
      return(0);
      break;

    case 'h':
      ttp_cli_print_help(argv[0]);
      return(0);
      break;

    case 'i':
      ttp_cli_args.infile1 = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      strcpy(ttp_cli_args.infile1, optarg);
      break;
    case 'I':
      ttp_cli_args.infile2 = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      strcpy(ttp_cli_args.infile2, optarg);
      break;

    case 'o':
      ttp_cli_args.outfile1 = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      strcpy(ttp_cli_args.outfile1, optarg);
      break;
    case 'O':
      ttp_cli_args.outfile2 = (char *)malloc((strlen(optarg)+1)*sizeof(char));
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
  printf("Bin time: %" PRIu64 "ps\n", ttp_cli_args.bin_time);
  printf("Window time: %" PRIu64 "ps\n", ttp_cli_args.window_time);
  printf("Block size: %d records\n", ttp_cli_args.block_size);
}  
