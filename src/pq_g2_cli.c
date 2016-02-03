#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <inttypes.h>

#include "scitoll.h"
#include "ttd.h"
#include "pq_g2_cli.h"

static const struct option pq_g2_longopts[] = {
        { "version", no_argument, NULL, 'V' },
        { "help", no_argument, NULL, 'h' },

        { "verbose", no_argument, NULL, 'v' },

        { "normalize", no_argument, NULL, 'N' },
        { "int-time", required_argument, NULL, 't' },

        { "delay", required_argument, NULL, 'd'},

        { "input-file", required_argument, NULL, 'i' },
        { "output-file", required_argument, NULL, 'o' },

        { "bin-time", required_argument, NULL, 'b' },
        { "window-time", required_argument, NULL, 'w' },
        { "input2-offset", required_argument, NULL, 'T' },

        { "block-size", required_argument, NULL, 'B' },
        { "ringbuffer-size", required_argument, NULL, 'R' },
};

static const char *pq_g2_optstring = "VhvNt:i:d:o:T:b:w:B:R:";

void pq_g2_cli_print_help(char* program_name) {
  // Need a string of spaces equal in length to the program name
  size_t len = strlen(program_name)+1;
  char pn_spaces[len];
  int i;
  for (i=0; i<len-1; i++) {
    pn_spaces[i] = ' ';
  }
  pn_spaces[len-1] = '\0';
  printf("Usage: %s -i input_file -o output_file [-d <channel>:<delay>] [-b bin_time]\n", program_name);
  printf("       %s [-w window_time] [-T input2_offset] [-t integration_time]\n", pn_spaces);
  //  printf("       %s [-B block_size]\n", pn_spaces);

  printf("\tNotes: \n");
//  printf("\t\t-1 (--channel1):\tFirst channel number (0-indexed)\n");
//  printf("\t\t-2 (--channel2):\tSecond channel number (0-indexed)\n");
  printf("\t\t-b (--bin-time):\tSpecified in picoseconds\n");
  printf("\t\t-w (--window-time):\tWindow time in picoseconds\n");
  //  printf("\t\t-B (--block-size):      Number of photon records to read into RAM at a time.\n");
  //  printf("\t\t                        Experimental option; don't change unless you have a good reason.\n");
  printf("\t\t-d (--delay): Delay a channel. Format: <0-indexed channel #>:<delay time>\n");
  printf("\t\t-T (--input-offset):\tOffset input2 relative to input1 (input in picoseconds)\n");
  printf("\t\t-N (--normalize):\tNormalize output histogram\n");
  printf("\t\t-t (--int-time):\tTotal integration time (input in seconds).\n");
  printf("\t\t\t\t\tIf normalize is used and this isn't set, will be approximated\n");
  printf("\t\t\t\t\tusing latest arrival time seen.\n");

  printf("\tOther options:\n");
  printf("\t\t-v (--verbose):\t\tEnable verbose output to stdout\n");
  printf("\t\t-h (--help):\t\tPrint this help dialog\n");
  printf("\t\t-V (--version):\t\tPrint program version\n");
}

int16_t parse_delay(char* input, int16_t *channel, int64_t *delay) {
  size_t nCharInput, nCharChannel, nCharDelay;
  int16_t i, colonIdx;

  nCharInput = strlen(input);
  // Find ':'
  colonIdx = -1;
  for (i=0; i < nCharInput; i++) {
    if (input[i] == ':') { colonIdx = i; }
  }

  if (colonIdx == -1) { return colonIdx; }

  nCharChannel = (size_t)colonIdx;
  nCharDelay = nCharInput - nCharChannel - 1;

  char* channelStr = (char*) calloc((size_t)nCharChannel + 1, sizeof(char));
  char* delayStr = (char*) calloc((size_t)nCharDelay + 1, sizeof(char));
  strncpy(channelStr, input, nCharChannel);
  strncpy(delayStr, input + colonIdx + 1, nCharDelay);
  *channel = (int16_t)atoi(channelStr);
  int retcode;
  *delay = scitoll(delayStr, &retcode);

  free(channelStr);
  free(delayStr);
  return 0;
}

int pq_g2_read_cli(int argc, char* argv[]) {
  int i, retcode=0;
  // Initialize default values
  pq_g2_cli_args.verbose = 0;
  pq_g2_cli_args.normalize = 0;
  pq_g2_cli_args.int_time = 0;

  int bin_time_set = 0;
  pq_g2_cli_args.bin_time = 10;

  int window_time_set = 0;
  pq_g2_cli_args.window_time = 10000;
  pq_g2_cli_args.channel2_offset = 0;

  pq_g2_cli_args.block_size = 16384;
  pq_g2_cli_args.rb_size = 1024;

  pq_g2_cli_args.infile_allocated = 0;
  pq_g2_cli_args.outfile_prefix_allocated = 0;

  for(i=0; i<PQ_HH_MAX_CHANNELS; i++) {
    pq_g2_cli_args.channel_offset[i] = 0;
    pq_g2_cli_args.channel_active[i] = 1;
  }

  int16_t channel;
  int64_t delay;

  // Read command line options
  int option_index, opt;
  opt = getopt_long(argc, argv, pq_g2_optstring, pq_g2_longopts, &option_index);
  while (opt != -1) {
    switch (opt) {
      case 'd':
        parse_delay(optarg, &channel, &delay);
        pq_g2_cli_args.channel_offset[channel] = delay;
        break;

      case 'v':
        pq_g2_cli_args.verbose = 1;
        break;

      case 'V':
        ttd_print_version(argv[0]);
        return(PQ_G2_CLI_EXIT_RETCODE);
        //break;

      case 'N':
        pq_g2_cli_args.normalize = 1;
        break;

      case 't':
        pq_g2_cli_args.int_time = (uint64_t)scitoll(optarg, &retcode);
        break;

      case 'h':
        pq_g2_cli_print_help(argv[0]);
        return(PQ_G2_CLI_EXIT_RETCODE);
        break;

      case 'i':
        pq_g2_cli_args.infile = (char *)malloc((strlen(optarg)+1)*sizeof(char));
        pq_g2_cli_args.infile_allocated = 1;
        strcpy(pq_g2_cli_args.infile, optarg);
        break;

      case 'o':
        pq_g2_cli_args.outfile_prefix = (char *)malloc((strlen(optarg)+1)*sizeof(char));
        pq_g2_cli_args.outfile_prefix_allocated = 1;
        strcpy(pq_g2_cli_args.outfile_prefix, optarg);
        break;

      case 'b':
        pq_g2_cli_args.bin_time = scitoll(optarg, &retcode);
        bin_time_set = 1;
        break;
      case 'w':
        pq_g2_cli_args.window_time = scitoll(optarg, &retcode);
        window_time_set = 1;
        break;
      case 'T':
        pq_g2_cli_args.channel2_offset = scitoll(optarg, &retcode);

      case 'B':
        pq_g2_cli_args.block_size = scitoll(optarg, &retcode);
        break;

      case 'R':
        pq_g2_cli_args.rb_size = scitoll(optarg, &retcode);
        break;

      default:
        // Shouldn't actually get here
        break;
    }
    opt = getopt_long(argc, argv, pq_g2_optstring, pq_g2_longopts, &option_index);
  }

  if (!(bin_time_set))
    fprintf(stderr, "Warning: Bin time not specified. Using default value of %" PRIu64 " ps\n", pq_g2_cli_args.bin_time);

  if (!(window_time_set))
    fprintf(stderr, "Warning: Window time not specified. Using default value of %" PRIu64 " ps\n", pq_g2_cli_args.window_time);

  return(0);
}

void pq_g2_print_options(int no_verbose) {
  printf("***Options Summary***\n");

  if (!(no_verbose)) {
    printf("Verbose: %d\n", pq_g2_cli_args.verbose);
  }

  if (pq_g2_cli_args.infile_allocated) {
    printf("Input File: %s\n", pq_g2_cli_args.infile);
  }

  if (pq_g2_cli_args.outfile_prefix != NULL) {
    printf("Output File: %s\n", pq_g2_cli_args.outfile_prefix);
  }

  printf("Bin time: %" PRIu64 " ps\n", pq_g2_cli_args.bin_time);
  printf("Window time: %" PRIu64 " ps\n", pq_g2_cli_args.window_time);
  printf("Offset file 2 times by %" PRId64 " ps\n", pq_g2_cli_args.channel2_offset);
  printf("Block size: %d records\n", pq_g2_cli_args.block_size);
  int i;
  for (i=0; i<PQ_HH_MAX_CHANNELS; i++) {
    if (pq_g2_cli_args.channel_offset[i] != 0) {
      printf("Delay Channel %d by %" PRId64 "ps\n", i, pq_g2_cli_args.channel_offset[i]);
    }
  }
}

void pq_g2_cli_cleanup() {
  if(pq_g2_cli_args.infile_allocated) {
    free(pq_g2_cli_args.infile);
    pq_g2_cli_args.infile_allocated = 0;
  }

  if(pq_g2_cli_args.outfile_prefix_allocated) {
    free(pq_g2_cli_args.outfile_prefix);
    pq_g2_cli_args.outfile_prefix_allocated = 0;
  }


}
