#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <inttypes.h>

#include "sci_to_int64.h"
#include "ttd_g4_cli.h"

static const struct option ttd_g4_longopts[] = {
        {"version",     no_argument,       NULL, 'V'},
        {"help",        no_argument,       NULL, 'h'},

        {"verbose",     no_argument,       NULL, 'v'},

        {"input1",      required_argument, NULL, '1'},
        {"input2",      required_argument, NULL, '2'},
        {"input3",      required_argument, NULL, '3'},
        {"input4",      required_argument, NULL, '4'},

        {"output-file", required_argument, NULL, 'o'},

        {"bin-time",    required_argument, NULL, 'b'},
        {"window-time", required_argument, NULL, 'w'},

        {"block-size",  required_argument, NULL, 'B'},
};

static const char *ttd_g4_optstring = "Vhv1:2:3:4:o:T:b:w:B:";

void g4_cli_print_help(char *program_name) {
  // Need a string of spaces equal in length to the program name
  size_t len = strlen(program_name) + 1;
  char pn_spaces[len];
  int i;
  for (i = 0; i < len - 1; i++) {
    pn_spaces[i] = ' ';
  }
  pn_spaces[len - 1] = '\0';
  printf("Usage: %s [-1 in1.ttd] [-2 in2.ttd] [-3 in3.ttd] [-4 in4.ttd]\n", program_name);
  printf("       %s [-o out.csv] [-b bin_time] [-w window_time]\n", pn_spaces);

  printf("\tNotes: \n");
  printf("\t\t-b (--bin-time):\tSpecified in picoseconds\n");
  printf("\t\t-w (--window-time):\tWindow time in picoseconds\n");

  printf("\tOther options:\n");
  printf("\t\t-v (--verbose):\t\tEnable verbose output to stdout\n");
  printf("\t\t-h (--help):\t\tPrint this help dialog\n");
  printf("\t\t-V (--version):\t\tPrint program version\n");
}

int g4_read_cli(int argc, char *argv[]) {
  // Initialize default values
  g4_cli_args.verbose = 0;

  int bin_time_set = 0;
  g4_cli_args.bin_time = 10;

  int window_time_set = 0;
  g4_cli_args.window_time = 10000;
  g4_cli_args.block_size = 16384;

  int i;
  for (i = 0; i < 4; i++) {
    g4_cli_args.infiles[i] = NULL;
  }
  g4_cli_args.outfile = NULL;

  int retcode = 0;

  // Read command line options
  int option_index, opt;
  opt = getopt_long(argc, argv, ttd_g4_optstring, ttd_g4_longopts, &option_index);
  while (opt != -1) {
    switch (opt) {
      case 'v':
        g4_cli_args.verbose = 1;
        break;

      case 'V':
        ttd_print_version(argv[0]);
        return (G4_CLI_EXIT_RETCODE);

      case 'h':
        g4_cli_print_help(argv[0]);
        return (G4_CLI_EXIT_RETCODE);

      case '1':
        g4_cli_args.infiles[0] = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
        strcpy(g4_cli_args.infiles[0], optarg);
        break;
      case '2':
        g4_cli_args.infiles[1] = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
        strcpy(g4_cli_args.infiles[1], optarg);
        break;

      case '3':
        g4_cli_args.infiles[2] = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
        strcpy(g4_cli_args.infiles[2], optarg);
        break;

      case '4':
        g4_cli_args.infiles[3] = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
        strcpy(g4_cli_args.infiles[3], optarg);
        break;

      case 'o':
        g4_cli_args.outfile = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
        strcpy(g4_cli_args.outfile, optarg);
        break;

      case 'b':
        g4_cli_args.bin_time = (ttd_t) sci_to_int64(optarg, &retcode);
        bin_time_set = 1;
        break;
      case 'w':
        g4_cli_args.window_time = (ttd_t) sci_to_int64(optarg, &retcode);
        window_time_set = 1;
        break;

      case 'B':
        g4_cli_args.block_size = (ttd_t) sci_to_int64(optarg, &retcode);
        break;

      default:
        // Shouldn't actually get here
        break;
    }
    opt = getopt_long(argc, argv, ttd_g4_optstring, ttd_g4_longopts, &option_index);
  }

  if (!(bin_time_set)) {
    fprintf(stderr, "Warning: Bin time not specified. Using default value of %" PRIu64 " ps\n", g4_cli_args.bin_time);
  }

  if (!(window_time_set)) {
    fprintf(stderr, "Warning: Window time not specified. Using default value of %" PRIu64 " ps\n",
            g4_cli_args.window_time);
  }

  return (0);
}

void g4_cli_print_options(int no_verbose) {
  int i;
  if (!(no_verbose)) {
    printf("Verbose: %d\n", g4_cli_args.verbose);
  }
  for (i = 0; i < 4; i++) {
    if (g4_cli_args.infiles[i] != NULL) {
      printf("Infile %d: %s\n", i + 1, g4_cli_args.infiles[i]);
    }
  }

  if (g4_cli_args.outfile != NULL) {
    printf("Outfile: %s\n", g4_cli_args.outfile);
  }

  printf("Bin time: %" PRIu64 " ps\n", g4_cli_args.bin_time);
  printf("Window time: %" PRIu64 " ps\n", g4_cli_args.window_time);
  printf("Block size: %lu records\n", g4_cli_args.block_size);
}

void g4_cli_cleanup() {
  int i;
  for (i = 0; i < 4; i++) {
    free(g4_cli_args.infiles[i]);
    g4_cli_args.infiles[i] = NULL;
  }

  free(g4_cli_args.outfile);
  g4_cli_args.outfile = NULL;
}
