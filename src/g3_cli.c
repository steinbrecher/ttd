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

#include "scitollu.h"
#include "g3_cli.h"

static const struct option ttd_g3_longopts[] = {
  { "version", no_argument, NULL, 'V' },
  { "help", no_argument, NULL, 'h' },

  { "verbose", no_argument, NULL, 'v' },

  { "input1", required_argument, NULL, '1' },
  { "input2", required_argument, NULL, '2' },
  { "input3", required_argument, NULL, '3' },

  { "output-file", required_argument, NULL, 'o' },

  { "bin-time", required_argument, NULL, 'b' },
  { "window-time", required_argument, NULL, 'w' },

  { "block-size", required_argument, NULL, 'B' },
};

static const char *ttd_g3_optstring = "Vhv1:2:3:o:T:b:w:B:";

void g3_cli_print_help(char* program_name) {
  // Need a string of spaces equal in length to the program name
  int len = strlen(program_name)+1;
  char pn_spaces[len];
  int i;
  for (i=0; i<len-1; i++) {
      pn_spaces[i] = ' ';
    }
  pn_spaces[len-1] = '\0';
  printf("Usage: %s [-1 in1.ttd] [-2 in2.ttd] [-3 in3.ttd] [-o output_file]\n", program_name);
  printf("       %s [-b bin_time] [-w window_time]\n", pn_spaces);

  printf("\tNotes: \n");
  printf("\t\t-b (--bin-time):\tSpecified in picoseconds\n");
  printf("\t\t-w (--window-time):\tWindow time in picoseconds\n");

  printf("\tOther options:\n");
  printf("\t\t-v (--verbose):\t\tEnable verbose output to stdout\n");
  printf("\t\t-h (--help):\t\tPrint this help dialog\n");
  printf("\t\t-V (--version):\t\tPrint program version\n");
}

int g3_read_cli(int argc, char* argv[]) {
  // Initialize default values
  g3_cli_args.verbose = 0;

  int bin_time_set = 0;
  g3_cli_args.bin_time = 10;

  int window_time_set = 0;
  g3_cli_args.window_time = 10000;

  g3_cli_args.block_size = 16384;

  g3_cli_args.infiles_allocated[0] = 0;
  g3_cli_args.infiles_allocated[1] = 0;
  g3_cli_args.infiles_allocated[2] = 0;
  g3_cli_args.outfile_allocated = 0;

  // Read command line options
  int option_index, opt;
  opt = getopt_long(argc, argv, ttd_g3_optstring, ttd_g3_longopts, &option_index);
  while (opt != -1) {
    switch (opt) {
    case 'v':
      g3_cli_args.verbose = 1;
      break;

    case 'V':
      ttd_print_version(argv[0]);
      return(G3_CLI_EXIT_RETCODE);
      break;

    case 'h':
      g3_cli_print_help(argv[0]);
      return(G3_CLI_EXIT_RETCODE);
      break;

    case '1':
      g3_cli_args.infile1 = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      g3_cli_args.infiles_allocated[0] = 1;
      strcpy(g3_cli_args.infile1, optarg);
      break;
    case '2':
      g3_cli_args.infile2 = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      g3_cli_args.infiles_allocated[1] = 1;
      strcpy(g3_cli_args.infile2, optarg);
      break;

    case '3':
      g3_cli_args.infile3 = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      g3_cli_args.infiles_allocated[2] = 1;
      strcpy(g3_cli_args.infile3, optarg);
      break;

    case 'o':
      g3_cli_args.outfile = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      g3_cli_args.outfile_allocated = 1;
      strcpy(g3_cli_args.outfile, optarg);
      break;

    case 'b':
      g3_cli_args.bin_time = scitollu(optarg);
      bin_time_set = 1;
      break;
    case 'w':
      g3_cli_args.window_time = scitollu(optarg);
      window_time_set = 1;
      break;

    case 'B':
      g3_cli_args.block_size = scitollu(optarg);
      break;

    default:
      // Shouldn't actually get here
      break;
    }
    opt = getopt_long(argc, argv, ttd_g3_optstring, ttd_g3_longopts, &option_index);
  }

  if (!(bin_time_set))
    fprintf(stderr, "Warning: Bin time not specified. Using default value of %" PRIu64 " ps\n", g3_cli_args.bin_time);

  if (!(window_time_set))
    fprintf(stderr, "Warning: Window time not specified. Using default value of %" PRIu64 " ps\n", g3_cli_args.window_time);

  return(0);
}

void g3_cli_print_options(int no_verbose) {
  if (!(no_verbose)) {
    printf("Verbose: %d\n", g3_cli_args.verbose);
  }
  if (g3_cli_args.infile1 != NULL) {
    printf("Infile 1: %s\n", g3_cli_args.infile1);
  }
  if (g3_cli_args.infile2 != NULL) {
    printf("Infile 2: %s\n", g3_cli_args.infile2);
  }
  if (g3_cli_args.infile3 != NULL) {
    printf("Infile 2: %s\n", g3_cli_args.infile3);
  }
  if (g3_cli_args.outfile != NULL) {
    printf("Outfile: %s\n", g3_cli_args.outfile);
  }
  printf("Bin time: %" PRIu64 " ps\n", g3_cli_args.bin_time);
  printf("Window time: %" PRIu64 " ps\n", g3_cli_args.window_time);
  printf("Block size: %d records\n", g3_cli_args.block_size);
}  

void g3_cli_cleanup() {
  if(g3_cli_args.infiles_allocated[0]) {
    free(g3_cli_args.infile1);
    g3_cli_args.infiles_allocated[0] = 0;
  }
  if(g3_cli_args.infiles_allocated[1]) {
    free(g3_cli_args.infile2);
    g3_cli_args.infiles_allocated[1] = 0;
  }
  if(g3_cli_args.infiles_allocated[2]) {
    free(g3_cli_args.infile3);
    g3_cli_args.infiles_allocated[2] = 0;
  }


  if(g3_cli_args.outfile_allocated) {
    free(g3_cli_args.outfile);
    g3_cli_args.outfile_allocated = 0;
  }

					  
}
