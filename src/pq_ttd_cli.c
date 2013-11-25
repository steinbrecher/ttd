#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include "ttp.h"
#include "ttd.h"
#include "pq_parse.h"
#include "pq_records.h"
#include "pq_ttd_cli.h"

static const struct option pq_ttd_longopts[] = {
  { "version", no_argument, NULL, 'V' },
  { "help", no_argument, NULL, 'h' },

  { "verbose", no_argument, NULL, 'v' },

  { "input-file", required_argument, NULL, 'i' },

  { "output-prefix", required_argument, NULL, 'o' },

  { "block-size", required_argument, NULL, 'B' },
};

static const char *pq_ttd_optstring = "Vhvi:o:B:";

void pq_ttd_cli_print_help(char* program_name) {
  // Need a string of spaces equal in length to the program name
  int len = strlen(program_name) + 1;
  char pn_spaces[len];
  int i;
  for (i=0; i<len-1; i++) {
      pn_spaces[i] = ' ';
    }
  pn_spaces[len-1] = '\0';
  printf("Usage: %s -i input_file [-o output_prefix]\n", program_name);
  //  printf("       %s [-B block_size]\n", pn_spaces);

  printf("\tNotes: \n");
  printf("\t\t-o (--output-prefix):\tPrefix for .ttd output files.\n");
  printf("\t\t                     \tOptional; if not provided, will use input filename.\n");

  printf("\tOther options:\n");
  printf("\t\t-v (--verbose):\t\tEnable verbose output to stdout\n");
  printf("\t\t-h (--help):\t\tPrint this help dialog\n");
  printf("\t\t-V (--version):\t\tPrint program version\n");
}

int pq_ttd_read_cli(int argc, char* argv[]) {
  // Initialize default values
  pq_ttd_cli_args.verbose = 0;

  pq_ttd_cli_args.block_size = 16384;

  pq_ttd_cli_args.infile_allocated = 0;
  pq_ttd_cli_args.output_prefix_allocated = 0;

  // Read command line options
  int option_index, opt;
  opt = getopt_long(argc, argv, pq_ttd_optstring, pq_ttd_longopts, &option_index);
  while (opt != -1) {
    switch (opt) {
    case 'v':
      pq_ttd_cli_args.verbose = 1;
      break;

    case 'V':
      ttp_print_version(argv[0]);
      return(PQ_TTD_CLI_EXIT_RETCODE);
      break;

    case 'h':
      pq_ttd_cli_print_help(argv[0]);
      return(PQ_TTD_CLI_EXIT_RETCODE);
      break;

    case 'i':
      pq_ttd_cli_args.infile = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      pq_ttd_cli_args.infile_allocated = 1;
      strcpy(pq_ttd_cli_args.infile, optarg);
      break;

    case 'o':
      pq_ttd_cli_args.output_prefix = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      pq_ttd_cli_args.output_prefix_allocated = 1;
      strcpy(pq_ttd_cli_args.output_prefix, optarg);
      break;

    case 'B':
      pq_ttd_cli_args.block_size = atoi(optarg);
      break;

    default:
      // Shouldn't actually get here
      break;
    }
    opt = getopt_long(argc, argv, pq_ttd_optstring, pq_ttd_longopts, &option_index);
  }
  return(0);
}

void pq_ttd_print_options(int no_verbose) {
  if (!(no_verbose)) {
    printf("Verbose: %d\n", pq_ttd_cli_args.verbose);
  }
  if (pq_ttd_cli_args.infile != NULL) {
    printf("Infile: %s\n", pq_ttd_cli_args.infile);
  }
  if (pq_ttd_cli_args.output_prefix != NULL) {
    printf("Output Prefix: %s\n", pq_ttd_cli_args.output_prefix);
  }
  printf("Block size: %d records\n", pq_ttd_cli_args.block_size);
}  

void pq_ttd_cli_cleanup() {
  if(pq_ttd_cli_args.infile_allocated) {
    free(pq_ttd_cli_args.infile);
    pq_ttd_cli_args.infile_allocated = 0;
  }

  if(pq_ttd_cli_args.output_prefix_allocated) {
    free(pq_ttd_cli_args.output_prefix);
    pq_ttd_cli_args.output_prefix_allocated = 0;
  }
}
