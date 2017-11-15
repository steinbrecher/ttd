#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>

#include "ttd.h"
#include "ttd_filebuffer.h"
#include "sci_to_int64.h"

#define TTD_SHIFT_EXIT 1

struct {
    int verbose; // -v
    char *infile; // -i
    char *outfile; // -o
    int64_t delay;
} ttd_shift_cli_args;

static const struct option ttd_shift_longopts[] = {
        {"version", no_argument,       NULL, 'V'},
        {"help",    no_argument,       NULL, 'h'},

        {"verbose", no_argument,       NULL, 'v'},

        {"input",   required_argument, NULL, 'i'},
        {"output",  required_argument, NULL, 'o'},

        {"offset",  required_argument, NULL, 'T'},
};

static const char *ttd_shift_optstring = "Vhvi:o:T:";

void ttd_shift_print_help(char *program_name) {
  size_t len = strlen(program_name) + 1;
  char pn_spaces[len];
  int i;
  for (i = 0; i < len - 1; i++) {
    pn_spaces[i] = ' ';
  }
  pn_spaces[len - 1] = '\0';
  printf("Usage: %s [-i infile] [-o outfile] [-T offset_time]\n", program_name);

  printf("\tNotes: \n");
  printf("\t\t-T (--offset):\tAmount to offset data in picoseconds. This must be positive.\n");
  printf("\t\t              \tNote that this option supports scientific notation (e.g. 5e4 or -3e2)\n");

  printf("\tOther options:\n");
  printf("\t\t-v (--verbose):\tEnable verbose output to stdout\n");
  printf("\t\t-h (--help):\tPrint this help dialog\n");
  printf("\t\t-V (--version):\tPrint program version\n");
}

int ttd_shift_read_cli(int argc, char *argv[]) {
  int retcode = 0;
  ttd_shift_cli_args.verbose = 0;
  ttd_shift_cli_args.infile = NULL;
  ttd_shift_cli_args.outfile = NULL;
  ttd_shift_cli_args.delay = 0;

  // Parse command line
  int option_index, opt;
  opt = getopt_long(argc, argv, ttd_shift_optstring, ttd_shift_longopts, &option_index);
  while (opt != -1) {
    switch (opt) {
      case 'v':
        ttd_shift_cli_args.verbose = 1;
        break;

      case 'V':
        ttd_print_version(argv[0]);
        return (TTD_SHIFT_EXIT);

      case 'h':
        ttd_shift_print_help(argv[0]);
        return (TTD_SHIFT_EXIT);

      case 'i':
        ttd_shift_cli_args.infile = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
        strcpy(ttd_shift_cli_args.infile, optarg);
        break;

      case 'o':
        ttd_shift_cli_args.outfile = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
        strcpy(ttd_shift_cli_args.outfile, optarg);
        break;

      case 'T':
        ttd_shift_cli_args.delay = sci_to_int64(optarg, &retcode);
        if (retcode < 0) {
          switch (retcode) {
            case SCITOLL_MULTIPLE_E:
              printf("Error: Delay is not a number\n");
              break;
            case SCITOLL_NO_MANTISSA:
              printf("Error: Delay missing mantissa\n");
              break;
            case SCITOLL_NO_EXPONENT:
              printf("Error: Delay missing exponent\n");
              break;
            case SCITOLL_NEGATIVE:
              printf("Error: Delay must be positive\n");
              break;
            case SCITOLL_DECIMAL:
              printf("Error: Delay may not have a decimal\n");
              break;
            default:
              printf("Error in parsing time delay\n");
          }
          goto cli_return;
        }
      default:
        break;
    }
    opt = getopt_long(argc, argv, ttd_shift_optstring, ttd_shift_longopts, &option_index);
  }
  cli_return:
  return (retcode);
}


int main(int argc, char *argv[]) {
  int retcode, exitcode = 0;

  retcode = ttd_shift_read_cli(argc, argv);
  if (retcode == TTD_SHIFT_EXIT) {
    goto cleanup_cli;
  } else if (retcode < 0) {
    exitcode = retcode;
    goto cleanup_cli;
  }

  int outfile_open = 0;
  FILE *outfile;
  outfile = fopen(ttd_shift_cli_args.outfile, "wb");
  outfile_open = 1;


  printf("Input file: %s\n", ttd_shift_cli_args.infile);
  printf("Output file: %s\n", ttd_shift_cli_args.outfile);
  printf("Delay Time: %" PRId64 " ps\n", ttd_shift_cli_args.delay);

  ttd_fb_t fb;
  ttd_fb_init(&fb, 16384, ttd_shift_cli_args.infile, ttd_shift_cli_args.delay);

  ttd_t time = ttd_fb_pop(&fb);
  if (((int64_t) time + ttd_shift_cli_args.delay) < 0) {
    fprintf(stderr, "Error: Time shift would make first record of %s negative.\n", ttd_shift_cli_args.outfile);
    fprintf(stderr, "Maximum negative timeshift is -%" PRIu64 " picoseconds\n", time);
    goto cleanup_fb;
  }

  while (!(fb.empty)) {
    fwrite(&time, sizeof(ttd_t), 1, outfile);
    time = ttd_fb_pop(&fb);
  }

  cleanup_fb:
  ttd_fb_cleanup(&fb);

  fclose(outfile);

  cleanup_cli:
  free(ttd_shift_cli_args.infile);
  ttd_shift_cli_args.infile = NULL;

  free(ttd_shift_cli_args.outfile);
  ttd_shift_cli_args.outfile = NULL;

  exit(exitcode);
}
  
  
  
