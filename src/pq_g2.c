//
// Created by Greg Steinbrecher on 1/31/16.
//

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>

#include "pq_g2.h"

int main(int argc, char* argv[]) {
  int retcode, exitcode=0;

  retcode = ttd_g2_read_cli(argc, argv);

  if (retcode < 0) {
    exitcode = retcode;
    goto cleanup_ttd_g2_cli;
  }
  else if (retcode == TTD_G2_CLI_EXIT_RETCODE) {
    goto cleanup_ttd_g2_cli;
  }

  if (ttd_g2_cli_args.verbose) {
    ttd_g2_print_options(TTD_G2_PRINTOPTIONS_NOVERBOSE);
  }
  char *infile1 = ttd_g2_cli_args.infile1;
  char *infile2 = ttd_g2_cli_args.infile2;
  char *outfile = ttd_g2_cli_args.outfile;


  if (infile1 == NULL) {
    printf("Error: Missing input file 1. Please specify with the '-1' flag.\n");
    exitcode = -1;
    goto cleanup_ttd_g2_cli;
  }
  if (infile2 == NULL) {
    printf("Error: Missing input file 2. Please specify with the '-2' flag.\n");
    exitcode = -1;
    goto cleanup_ttd_g2_cli;
  }
  if (outfile == NULL) {
    printf("Error: Missing output file. Please specify with the '-o' flag.\n");
    exitcode = -1;
    goto cleanup_ttd_g2_cli;
  }


  ttd_ccorr2_t *g2_ccorr = ttd_g2(infile1, infile2, &retcode);

  ttd_ccorr2_write_csv(g2_ccorr, outfile, ttd_g2_cli_args.normalize, ttd_g2_cli_args.int_time);


  cleanup_ccorr:
  ttd_ccorr2_cleanup(g2_ccorr);
  free(g2_ccorr);

  cleanup_ttd_g2_cli:
  ttd_g2_cli_cleanup();

  exit_block:
  exit(exitcode);
}
