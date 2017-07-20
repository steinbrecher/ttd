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

#define TTD_DECIMATE_EXIT 1
#define TTD_DECIMATE_MISSING_FLAG -1

struct {
    int verbose; // -v
    char *infile; // -i
    char *outfile; // -o
    int64_t n; // -n
    int64_t offset;
} ttd_decimate_cli_args;

static const struct option ttd_decimate_longopts[] = {
        {"version", no_argument,       NULL, 'V'},
        {"help",    no_argument,       NULL, 'h'},

        {"verbose", no_argument,       NULL, 'v'},

        {"input",   required_argument, NULL, 'i'},
        {"output",  required_argument, NULL, 'o'},

        {"decimation_factor",  required_argument, NULL, 'n'},
        {"offset", required_argument, NULL, 'O'}
};

static const char *ttd_decimate_optstring = "Vhvi:o:n:O:";

void ttd_decimate_print_help(char *program_name) {
  size_t len = strlen(program_name) + 1;
  char pn_spaces[len];
  int i;
  for (i = 0; i < len - 1; i++) {
    pn_spaces[i] = ' ';
  }
  pn_spaces[len - 1] = '\0';
  printf("Usage: %s -i infile -o outfile -n decimation_factor [-O offset]\n", program_name);

  printf("\tNotes: \n");
  printf("\t\t-n (--decimation-factor): Output one record every n\n");
  printf("\t\t-O (--offset):            Offset by some number of records (default: 0) \n");

  printf("\tOther options:\n");
  printf("\t\t-v (--verbose):\tEnable verbose output to stdout\n");
  printf("\t\t-h (--help):\tPrint this help dialog\n");
  printf("\t\t-V (--version):\tPrint program version\n");
}

int ttd_decimate_read_cli(int argc, char *argv[]) {
  int retcode = 0;
  ttd_decimate_cli_args.verbose = 0;
  ttd_decimate_cli_args.infile = NULL;
  ttd_decimate_cli_args.outfile = NULL;
  ttd_decimate_cli_args.n = 0;
  ttd_decimate_cli_args.offset = 0;

  // Parse command line
  int option_index, opt;
  opt = getopt_long(argc, argv, ttd_decimate_optstring, ttd_decimate_longopts, &option_index);
  while (opt != -1) {
    switch (opt) {
      case 'v':
        ttd_decimate_cli_args.verbose = 1;
        break;

      case 'V':
        ttd_print_version(argv[0]);
        return (TTD_DECIMATE_EXIT);

      case 'h':
        ttd_decimate_print_help(argv[0]);
        return (TTD_DECIMATE_EXIT);

      case 'i':
        ttd_decimate_cli_args.infile = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
        strcpy(ttd_decimate_cli_args.infile, optarg);
        break;

      case 'o':
        ttd_decimate_cli_args.outfile = (char *) malloc((strlen(optarg) + 1) * sizeof(char));
        strcpy(ttd_decimate_cli_args.outfile, optarg);
        break;

      case 'n':
        ttd_decimate_cli_args.n = (int64_t)atol(optarg);
        break;

      case 'O':
        ttd_decimate_cli_args.offset = (int64_t)atol(optarg);
        break;

      default:
        break;
    }
    opt = getopt_long(argc, argv, ttd_decimate_optstring, ttd_decimate_longopts, &option_index);
  }
  if ((ttd_decimate_cli_args.n == 0) ||
      (ttd_decimate_cli_args.infile == NULL) ||
      (ttd_decimate_cli_args.outfile == NULL)) {
    return(TTD_DECIMATE_MISSING_FLAG);
  }

  return (retcode);
}

int main(int argc, char *argv[]) {
  int retcode, exitcode = 0;
  size_t i;

  retcode = ttd_decimate_read_cli(argc, argv);
  if (retcode == TTD_DECIMATE_EXIT) {
    goto cleanup_cli;
  } else if (retcode < 0) {
    exitcode = retcode;
    if (retcode == TTD_DECIMATE_MISSING_FLAG) {
      fprintf(stderr, "Error: Missing required flag; you need at least -i, -o, and -n\n");
    }
    goto cleanup_cli;
  }
  
  int outfile_open = 0;
  FILE *outfile;
  outfile = fopen(ttd_decimate_cli_args.outfile, "wb");
  outfile_open = 1;

  printf("Input file: %s\n", ttd_decimate_cli_args.infile);
  printf("Output file: %s\n", ttd_decimate_cli_args.outfile);
  printf("Decimation factor: %" PRId64 "\n", ttd_decimate_cli_args.n);
  printf("Offset: %" PRId64 "\n", ttd_decimate_cli_args.offset);

  ttd_fb_t fb;
  ttd_fb_init(&fb, 16384, ttd_decimate_cli_args.infile, 0);

  ttd_t time;

  // Skip offset number of records
  for (i=0; i<ttd_decimate_cli_args.offset; i++) {
    if (fb.empty) {break;}
    ttd_fb_pop(&fb);
  }

  if (fb.empty) {
    fprintf(stderr, "Error: offset larger than total number of records\n");
    goto cleanup_fb;
  }

  while (!(fb.empty)) {
    time = ttd_fb_pop(&fb);
    // Output first record
    fwrite(&time, sizeof(ttd_t), 1, outfile);
    // Skip n-1 records
    for (i=0; i < ttd_decimate_cli_args.n-1; i++) {
      // Make sure to check before each pop whether the filebuffer is empty
      if (fb.empty) {break;}
      ttd_fb_pop(&fb);
    }
  }

  cleanup_fb:
  ttd_fb_cleanup(&fb);

  fclose(outfile);


  cleanup_cli:
  free(ttd_decimate_cli_args.infile);
  ttd_decimate_cli_args.infile = NULL;

  free(ttd_decimate_cli_args.outfile);
  ttd_decimate_cli_args.outfile = NULL;

  exit(exitcode);

}