#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include "scitoll.h"
#include "ttd.h"
#include "ttd_g2_cli.h"

static const struct option ttd_g2_longopts[] = {
  { "version", no_argument, NULL, 'V' },
  { "help", no_argument, NULL, 'h' },

  { "verbose", no_argument, NULL, 'v' },

  { "input1", required_argument, NULL, '1' },
  { "input2", required_argument, NULL, '2' },

  { "output-file", required_argument, NULL, 'o' },

  { "bin-time", required_argument, NULL, 'b' },
  { "window-time", required_argument, NULL, 'w' },
  { "input2-offset", required_argument, NULL, 'T' },

  { "block-size", required_argument, NULL, 'B' },
};

static const char *ttd_g2_optstring = "Vhv1:2:o:T:b:w:B:";

void ttd_g2_cli_print_help(char* program_name) {
  // Need a string of spaces equal in length to the program name
  int len = strlen(program_name)+1;
  char pn_spaces[len];
  int i;
  for (i=0; i<len-1; i++) {
      pn_spaces[i] = ' ';
    }
  pn_spaces[len-1] = '\0';
  printf("Usage: %s [-1 input_file_1] [-2 input_file_2] [-o output_file_1]\n", program_name);
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

int ttd_g2_read_cli(int argc, char* argv[]) {
  int retcode=0;
  // Initialize default values
  ttd_g2_cli_args.verbose = 0;

  int bin_time_set = 0;
  ttd_g2_cli_args.bin_time = 10;

  int window_time_set = 0;
  ttd_g2_cli_args.window_time = 10000;
  ttd_g2_cli_args.infile2_offset = 0;

  ttd_g2_cli_args.block_size = 16384;

  ttd_g2_cli_args.infiles_allocated[0] = 0;
  ttd_g2_cli_args.infiles_allocated[1] = 0;
  ttd_g2_cli_args.outfile_allocated = 0;

  // Read command line options
  int option_index, opt;
  opt = getopt_long(argc, argv, ttd_g2_optstring, ttd_g2_longopts, &option_index);
  while (opt != -1) {
    switch (opt) {
    case 'v':
      ttd_g2_cli_args.verbose = 1;
      break;

    case 'V':
      ttd_print_version(argv[0]);
      return(TTD_G2_CLI_EXIT_RETCODE);
      break;

    case 'h':
      ttd_g2_cli_print_help(argv[0]);
      return(TTD_G2_CLI_EXIT_RETCODE);
      break;

    case '1':
      ttd_g2_cli_args.infile1 = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      ttd_g2_cli_args.infiles_allocated[0] = 1;
      strcpy(ttd_g2_cli_args.infile1, optarg);
      break;
    case '2':
      ttd_g2_cli_args.infile2 = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      ttd_g2_cli_args.infiles_allocated[1] = 1;
      strcpy(ttd_g2_cli_args.infile2, optarg);
      break;

    case 'o':
      ttd_g2_cli_args.outfile = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      ttd_g2_cli_args.outfile_allocated = 1;
      strcpy(ttd_g2_cli_args.outfile, optarg);
      break;

    case 'b':
      ttd_g2_cli_args.bin_time = scitoll(optarg, &retcode);
      bin_time_set = 1;
      break;
    case 'w':
      ttd_g2_cli_args.window_time = scitoll(optarg, &retcode);
      window_time_set = 1;
      break;
    case 'T':
      ttd_g2_cli_args.infile2_offset = scitoll(optarg, &retcode);

    case 'B':
      ttd_g2_cli_args.block_size = scitoll(optarg, &retcode);
      break;

    default:
      // Shouldn't actually get here
      break;
    }
    opt = getopt_long(argc, argv, ttd_g2_optstring, ttd_g2_longopts, &option_index);
  }

  if (!(bin_time_set))
    fprintf(stderr, "Warning: Bin time not specified. Using default value of %" PRIu64 " ps\n", ttd_g2_cli_args.bin_time);

  if (!(window_time_set))
    fprintf(stderr, "Warning: Window time not specified. Using default value of %" PRIu64 " ps\n", ttd_g2_cli_args.window_time);

  return(0);
}

void ttd_g2_print_options(int no_verbose) {
  if (!(no_verbose)) {
    printf("Verbose: %d\n", ttd_g2_cli_args.verbose);
  }
  if (ttd_g2_cli_args.infile1 != NULL) {
    printf("Infile 1: %s\n", ttd_g2_cli_args.infile1);
  }
  if (ttd_g2_cli_args.infile2 != NULL) {
    printf("Infile 2: %s\n", ttd_g2_cli_args.infile2);
  }
  if (ttd_g2_cli_args.outfile != NULL) {
    printf("Outfile: %s\n", ttd_g2_cli_args.outfile);
  }
  printf("Bin time: %" PRIu64 " ps\n", ttd_g2_cli_args.bin_time);
  printf("Window time: %" PRIu64 " ps\n", ttd_g2_cli_args.window_time);
  printf("Offset file 2 times by %" PRId64 " ps\n", ttd_g2_cli_args.infile2_offset);
  printf("Block size: %d records\n", ttd_g2_cli_args.block_size);
}  

void ttd_g2_cli_cleanup() {
  if(ttd_g2_cli_args.infiles_allocated[0]) {
    free(ttd_g2_cli_args.infile1);
    ttd_g2_cli_args.infiles_allocated[0] = 0;
  }
  if(ttd_g2_cli_args.infiles_allocated[1]) {
    free(ttd_g2_cli_args.infile2);
    ttd_g2_cli_args.infiles_allocated[1] = 0;
  }

  if(ttd_g2_cli_args.outfile_allocated) {
    free(ttd_g2_cli_args.outfile);
    ttd_g2_cli_args.outfile_allocated = 0;
  }

					  
}
