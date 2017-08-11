#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <inttypes.h>
#include <locale.h>
#include <unistd.h>

#include "sci_to_int64.h"
#include "ttd.h"
#include "pq_gn_cli.h"

static const struct option pq_gn_longopts[] = {
        { "version", no_argument, NULL, 'V' },
        { "help", no_argument, NULL, 'h' },

        { "verbose", no_argument, NULL, 'v' },

        { "include-g", required_argument, NULL, 'g'},

        { "normalize", no_argument, NULL, 'N' },

        { "delay", required_argument, NULL, 'd'},

        { "input-file", required_argument, NULL, 'i' },
        { "output-prefix", required_argument, NULL, 'o' },

        { "bin-time", required_argument, NULL, 'b' },
        { "window-time", required_argument, NULL, 'w' },
        { "chunk-time", required_argument, NULL, 'c' },

        // Undocumented deliberately; for performance testing
        { "block-size", required_argument, NULL, 'B' },
        { "ringbuffer-size", required_argument, NULL, 'R' },
};

static const char *pq_gn_optstring = "Vhvg:Ni:d:o:T:b:w:c:B:R:";

void pq_gn_cli_print_help(char *program_name) {
  // Need a string of spaces equal in length to the program name
  size_t len = strlen(program_name)+1;
  char pn_spaces[len];
  int i;
  for (i=0; i<len-1; i++) {
    pn_spaces[i] = ' ';
  }
  pn_spaces[len-1] = '\0';
  printf("Usage: %s -g <2,3,4> -i input_file -o output_prefix [-d <chan>:<delay>]\n", program_name);// [-b bin_time] [-w window_time] [-N]\n", program_name);
  printf("       %s [-b bin_time] [-w window_time] [-N]\n", pn_spaces);
  //printf("       %s[-t integration_time]\n", pn_spaces);

  printf("\n");

  printf("\tFlags: \n");
  printf("\t\t-g (--include-g):\tCross-correlation order to include (2,3, or 4)\n");
  printf("\t\t                 \tNote: use flag multiple times for multiple orders\n");
  printf("\t\t                 \t(e.g. '-g 2 -g 3')\n");
  printf("\t\t-i (--input-file):\tInput file (.ht2, .ptu, or similar)\n");
  printf("\t\t-o (--output-prefix):\tString to prepend to output CSV files\n");
  printf("\t\t-d (--delay):\t\tDelay a channel. Format: <0-indexed channel #>:<delay time>\n");
  printf("\t\t-b (--bin-time):\tBin time in picoseconds [default: 10]\n");
  printf("\t\t-w (--window-time):\tWindow time in picoseconds [default: 1e4]\n");
  printf("\t\t-N (--normalize):\tNormalize output histograms (only supported for g(2) currently)\n");
  //printf("\t\t-t (--int-time):\tTotal integration time (input in seconds); only needed for normalization\n");
  //printf("\t\t\t\t\tIf normalize is used and this isn't set, will be approximated.\n");

  printf("\n");

  printf("\tOther options:\n");
  printf("\t\t-h (--help):\t\tPrint this help dialog\n");
  printf("\t\t-V (--version):\t\tPrint program version\n");

  printf("\n");

  printf("Examples:\n");
  printf("\tThis computes G(2) and G(3) cross-correlations with a bin size of 10ps, and a window size\n");
  printf("\tof 500ps from the file counts.ht2, saving results to CSV files w/ prefix 'results':\n\n");
  printf("\t\t$ %s -g 2 -g 3 -b 10 -w 500 -i counts.ht2 -o results\n", program_name);

  printf("\n");

  printf("\tThis is the same as Example 1, only computing G2 cross-correlation and delaying Channel 3 \n\tby 1838ps:\n\n");
  printf("\t\t$ %s -g 2 -b 10 -w 500 -d 3:1838 -i counts.ht2 -o results\n", program_name);

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
  int retcode = 0;

  *delay = sci_to_int64(delayStr, &retcode);

  if (retcode != 0) {
    sci_to_int64_printerr(delayStr, retcode);
    free(channelStr);
    free(delayStr);
    return(-1);
  }

  free(channelStr);
  free(delayStr);
  return 0;
}

int pq_gn_read_cli(int argc, char **argv) {
  int i, retcode=0;
  // Initialize default values
  pq_gn_cli_args.verbose = 1;
  pq_gn_cli_args.normalize = 0;
  pq_gn_cli_args.int_time = 0;

  int bin_time_set = 0;
  pq_gn_cli_args.bin_time = 10;

  int window_time_set = 0;
  pq_gn_cli_args.window_time = 10000;
  pq_gn_cli_args.channel2_offset = 0;

  pq_gn_cli_args.block_size = 16384;
  pq_gn_cli_args.rb_size = 1024;

  pq_gn_cli_args.chunk_time = 0;

  pq_gn_cli_args.infile = NULL;
  pq_gn_cli_args.outfile_prefix = NULL;

  for(i=0; i<PQ_HH_MAX_CHANNELS; i++) {
    pq_gn_cli_args.channel_offset[i] = 0;
    pq_gn_cli_args.channel_active[i] = 1;
  }

  for(i=0; i<= PQ_GN_MAX_CORRELATION_ORDER; i++) {
    pq_gn_cli_args.activeCorrelationOrders[i] = 0;
  }

  int16_t channel;
  int64_t delay;

  // Read command line options
  int option_index, opt;
  int64_t g;
  char end = '\0';
  char *pEnd = &end;
  opt = getopt_long(argc, argv, pq_gn_optstring, pq_gn_longopts, &option_index);
  while (opt != -1) {
    switch (opt) {
      case 'd':
        parse_delay(optarg, &channel, &delay);
        pq_gn_cli_args.channel_offset[channel] = delay;
        break;

      case 'g':
        g = strtol(optarg, &pEnd, 10);
        if (*pEnd) {
          fprintf(stderr, "Error: Cannot convert %s to a valid correlation order.\n", optarg);
          return(-1);
        }
        if ((g > 1) && (g <= PQ_GN_MAX_CORRELATION_ORDER)) {
          pq_gn_cli_args.activeCorrelationOrders[g] = 1;
        }
        else {
          fprintf(stderr, "Error: %" PRId64 " is not a valid correlation order.\n", g);
          return(-1);
        }
        break;


      case 'V':
        ttd_print_version(argv[0]);
        return(PQ_GN_CLI_EXIT_RETCODE);

      case 'N':
        pq_gn_cli_args.normalize = 1;
        break;

      case 't':
        pq_gn_cli_args.int_time = (uint64_t) sci_to_int64(optarg, &retcode);
        if (retcode != 0) {
          sci_to_int64_printerr(optarg, retcode);
          return(-1);
        }
        break;

      case 'h':
        pq_gn_cli_print_help(argv[0]);
        return(PQ_GN_CLI_EXIT_RETCODE);

      case 'i':
        if (strlen(optarg) > 4096) {
          fprintf(stderr, "Error: Input filename too long.\n");
          return(-1);
        }
        pq_gn_cli_args.infile = (char *)calloc((strlen(optarg)+1), sizeof(char));
        strcpy(pq_gn_cli_args.infile, optarg);
        break;

      case 'o':
        if (strlen(optarg) > 4096) {
          fprintf(stderr, "Error: Output prefix too long.\n");
          return(-1);
        }
        pq_gn_cli_args.outfile_prefix = (char *)calloc((strlen(optarg)+1), sizeof(char));
        strcpy(pq_gn_cli_args.outfile_prefix, optarg);
        break;

      case 'b':
        pq_gn_cli_args.bin_time = (ttd_t) sci_to_int64(optarg, &retcode);
        if (retcode != 0) {
          sci_to_int64_printerr(optarg, retcode);
          return(-1);
        }
        bin_time_set = 1;
        break;

      case 'w':
        pq_gn_cli_args.window_time = (ttd_t) sci_to_int64(optarg, &retcode);
        if (retcode != 0) {
          sci_to_int64_printerr(optarg, retcode);
          return(-1);
        }
        window_time_set = 1;
        break;

      case 'c':
        pq_gn_cli_args.chunk_time = (ttd_t) sci_to_int64(optarg, &retcode);
        if (retcode != 0) {
          sci_to_int64_printerr(optarg, retcode);
          return(-1);
        }
        break;

      case 'B':
        pq_gn_cli_args.block_size = (ttd_t) sci_to_int64(optarg, &retcode);
        if (retcode != 0) {
          sci_to_int64_printerr(optarg, retcode);
          return(-1);
        }
        break;

      case 'R':
        pq_gn_cli_args.rb_size = (ttd_t) sci_to_int64(optarg, &retcode);
        if (retcode != 0) {
          sci_to_int64_printerr(optarg, retcode);
          return(-1);
        }
        break;

      default:
        // Shouldn't actually get here
        break;
    }
    opt = getopt_long(argc, argv, pq_gn_optstring, pq_gn_longopts, &option_index);
  }

  if (!(bin_time_set))
    fprintf(stderr, "Warning: Bin time not specified. Using default value of %" PRIu64 " ps\n", pq_gn_cli_args.bin_time);

  if (!(window_time_set))
    fprintf(stderr, "Warning: Window time not specified. Using default value of %" PRIu64 " ps\n", pq_gn_cli_args.window_time);

  // Add two padding bins worth of extra time to compensate for edge effects
  pq_gn_cli_args.padded_window_time = pq_gn_cli_args.window_time + pq_gn_cli_args.bin_time;

  return(0);
}

int check_pq_gn_cli_args() {
  int i;
  // -V (--version)
  // -h (--help)
  // -v (--verbose)

  // -g (--include-g)
  // Test g1: Orders 0 or 1 set?
  if (pq_gn_cli_args.activeCorrelationOrders[0] || pq_gn_cli_args.activeCorrelationOrders[1]) {
    fprintf(stderr, "Warning: -g 0 and -g 1 do not do anything.\n");
    pq_gn_cli_args.activeCorrelationOrders[0] = 0;
    pq_gn_cli_args.activeCorrelationOrders[1] = 0;
  }

  // Test g2: Any order set?
  _Bool any_order_set = 0;
  for (i=2; i <= PQ_GN_MAX_CORRELATION_ORDER; i++) {
    if (pq_gn_cli_args.activeCorrelationOrders[i] == 1) {
      any_order_set = 1;
    }
  }
  if (!any_order_set) {
    fprintf(stderr, "Error: Please select at least one correlation order with the -g flag.\n");
    return(-1);
  }

  // -N (--normalize)
  // Test N1: Make sure only g(2) selected
  if (pq_gn_cli_args.normalize) {
    for (i = 3; i <= PQ_GN_MAX_CORRELATION_ORDER; i++) {
      if (pq_gn_cli_args.activeCorrelationOrders[i]) {
        fprintf(stderr, "Error: Histogram normalization only supported for g(2) in this version.\n");
        return (-1);
      }
    }
  }

  // -d (--delay)
  // -i (--input-file)
  // Test i1: Was an input file provided?
  if (pq_gn_cli_args.infile == NULL) {
    fprintf(stderr, "Error: Please provide an input file with the -i flag\n");
    return(-1);
  }
  // Test i2: Does file exist?
  if (access(pq_gn_cli_args.infile, F_OK ) == -1) {
    fprintf(stderr, "Error: Input file %s does not exist\n", pq_gn_cli_args.infile);
    return(-1);
  }

  // -o (--output-prefix)
  // -b (--bin-time)
  // -w (--window-time)
  // Test w1: Is window time > bin time?
  if (pq_gn_cli_args.bin_time >= pq_gn_cli_args.window_time) {
    fprintf(stderr, "Error: Window time must be greater than bin time\n");
    return(-1);
  }

  // All good
  return(0);
}

void pq_gn_print_options(int no_verbose) {
  setlocale(LC_NUMERIC, "");
  fprintf(stderr, KHEAD1 "Options Summary\n" KNRM);

  if (pq_gn_cli_args.infile != NULL) {
    fprintf(stderr, KHEAD2 "Input File:" KNRM KFILE " %s\n" KNRM, pq_gn_cli_args.infile);
  }

  if (pq_gn_cli_args.outfile_prefix != NULL) {
    fprintf(stderr, KHEAD2 "Output File Format: " KNRM KFILE "%s_" KITL "<chanA>" KFILE "-" KITL "<chanB>" KFILE ".csv\n" KNRM, pq_gn_cli_args.outfile_prefix);
  }

  fprintf(stderr, KHEAD2 "Bin time: " KNRM KTIME "%'" PRIu64 " ps\n" KNRM, pq_gn_cli_args.bin_time);
  fprintf(stderr, KHEAD2 "Window time: " KNRM KTIME "%'" PRIu64 " ps\n" KNRM, pq_gn_cli_args.window_time);
  if (pq_gn_cli_args.chunk_time > 0) {
    fprintf(stderr, KHEAD2 "Chunk time: " KNRM KTIME "%'" PRIu64 " ps\n" KNRM, pq_gn_cli_args.chunk_time);
  }


  int i;
  fprintf(stderr, "\n");
  for (i=0; i<PQ_HH_MAX_CHANNELS; i++) {
    if (pq_gn_cli_args.channel_offset[i] != 0) {
      fprintf(stderr, KHEAD2 KCYN "Channel %d: " KHEAD2 "Delay by " KNRM KTIME "%'" PRId64 " ps\n" KNRM, i, pq_gn_cli_args.channel_offset[i]);
    }
  }
}

void pq_gn_cli_cleanup() {
  free(pq_gn_cli_args.infile);
  pq_gn_cli_args.infile = NULL;

  free(pq_gn_cli_args.outfile_prefix);
  pq_gn_cli_args.outfile_prefix = NULL;
}
