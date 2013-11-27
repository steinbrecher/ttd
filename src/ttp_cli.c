#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include "scitollu.h"
#include "ttp_cli.h"

static const struct option ttp_longopts[] = {
  { "version", no_argument, NULL, 'V' },
  { "help", no_argument, NULL, 'h' },

  { "verbose", no_argument, NULL, 'v' },

  { "input1", required_argument, NULL, 'i' },
  { "input2", required_argument, NULL, 'I' },

  { "output-file", required_argument, NULL, 'o' },

  { "bin-time", required_argument, NULL, 'b' },
  { "window-time", required_argument, NULL, 'w' },
  { "input2-offset", required_argument, NULL, 'T' },

  { "block-size", required_argument, NULL, 'B' },
};

static const char *ttp_optstring = "Vhvi:I:o:T:b:w:B:";

void ttp_cli_print_help(char* program_name) {
  // Need a string of spaces equal in length to the program name
  int len = strlen(program_name)+1;
  char pn_spaces[len];
  int i;
  for (i=0; i<len-1; i++) {
      pn_spaces[i] = ' ';
    }
  pn_spaces[len-1] = '\0';
  printf("Usage: %s [-i input_file_1] [-I input_file_2] [-o output_file_1]\n", program_name);
  printf("       %s [-b bin_time] [-w window_time] [-T input2_offset]\n", pn_spaces);
  //  printf("       %s [-B block_size]\n", pn_spaces);

  printf("\tNotes: \n");
  printf("\t\t-b (--bin-time):\tSpecified in picoseconds\n");
  printf("\t\t-w (--window-time):\tWindow time in picoseconds\n");
  //  printf("\t\t-B (--block-size):      Number of photon records to read into RAM at a time.\n");
  //  printf("\t\t                        Experimental option; don't change unless you have a good reason.\n");
  printf("\t\t-T (--input-offset):\tOffset input2 relative to input1 (input in picoseconds)\n");

  printf("\tOther options:\n");
  printf("\t\t-v (--verbose):\t\tEnable verbose output to stdout\n");
  printf("\t\t-h (--help):\t\tPrint this help dialog\n");
  printf("\t\t-V (--version):\t\tPrint program version\n");
}

int ttp_read_cli(int argc, char* argv[]) {
  // Initialize default values
  ttp_cli_args.verbose = 0;

  int bin_time_set = 0;
  ttp_cli_args.bin_time = 10;

  int window_time_set = 0;
  ttp_cli_args.window_time = 10000;
  ttp_cli_args.infile2_offset = 0;

  ttp_cli_args.block_size = 16384;

  ttp_cli_args.infiles_allocated[0] = 0;
  ttp_cli_args.infiles_allocated[1] = 0;
  ttp_cli_args.outfile_allocated = 0;

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
      return(TTP_CLI_EXIT_RETCODE);
      break;

    case 'h':
      ttp_cli_print_help(argv[0]);
      return(TTP_CLI_EXIT_RETCODE);
      break;

    case 'i':
      ttp_cli_args.infile1 = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      ttp_cli_args.infiles_allocated[0] = 1;
      strcpy(ttp_cli_args.infile1, optarg);
      break;
    case 'I':
      ttp_cli_args.infile2 = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      ttp_cli_args.infiles_allocated[1] = 1;
      strcpy(ttp_cli_args.infile2, optarg);
      break;

    case 'o':
      ttp_cli_args.outfile = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      ttp_cli_args.outfile_allocated = 1;
      strcpy(ttp_cli_args.outfile, optarg);
      break;

    case 'b':
      ttp_cli_args.bin_time = scitollu(optarg);
      bin_time_set = 1;
      break;
    case 'w':
      ttp_cli_args.window_time = scitollu(optarg);
      window_time_set = 1;
      break;
    case 'T':
      ttp_cli_args.infile2_offset = atoll(optarg);

    case 'B':
      ttp_cli_args.block_size = scitollu(optarg);
      break;

    default:
      // Shouldn't actually get here
      break;
    }
    opt = getopt_long(argc, argv, ttp_optstring, ttp_longopts, &option_index);
  }

  if (!(bin_time_set))
    printf("Warning: Bin time not specified. Using default value of %" PRIu64 " ps\n", ttp_cli_args.bin_time);

  if (!(bin_time_set))
    printf("Warning: Window time not specified. Using default value of %" PRIu64 " ps\n", ttp_cli_args.window_time);

  return(0);
}

void ttp_print_options(int no_verbose) {
  if (!(no_verbose)) {
    printf("Verbose: %d\n", ttp_cli_args.verbose);
  }
  if (ttp_cli_args.infile1 != NULL) {
    printf("Infile 1: %s\n", ttp_cli_args.infile1);
  }
  if (ttp_cli_args.infile2 != NULL) {
    printf("Infile 2: %s\n", ttp_cli_args.infile2);
  }
  if (ttp_cli_args.outfile != NULL) {
    printf("Outfile: %s\n", ttp_cli_args.outfile);
  }
  printf("Bin time: %" PRIu64 " ps\n", ttp_cli_args.bin_time);
  printf("Window time: %" PRIu64 " ps\n", ttp_cli_args.window_time);
  printf("Offset file 2 times by %" PRId64 " ps\n", ttp_cli_args.infile2_offset);
  printf("Block size: %d records\n", ttp_cli_args.block_size);
}  

void ttp_cli_cleanup() {
  if(ttp_cli_args.infiles_allocated[0]) {
    free(ttp_cli_args.infile1);
    ttp_cli_args.infiles_allocated[0] = 0;
  }
  if(ttp_cli_args.infiles_allocated[1]) {
    free(ttp_cli_args.infile2);
    ttp_cli_args.infiles_allocated[1] = 0;
  }

  if(ttp_cli_args.outfile_allocated) {
    free(ttp_cli_args.outfile);
    ttp_cli_args.outfile_allocated = 0;
  }

					  
}
