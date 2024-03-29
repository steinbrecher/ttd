#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "ttd_g3_cli.h"

#include "ttd_crosscorr3.h"
#include "ttd_filebuffer.h"

ttd_ccorr3_t *ttd_g3(char *infile1, char *infile2, char *infile3, int *retcode) {

  uint64_t t1, t2, t3, ta, tb;

  ttd_ccorr3_t *ccorr = ttd_ccorr3_build(g3_cli_args.bin_time, g3_cli_args.window_time, 256);

  ttd_fb_t fb1, fb2, fb3;

  // Initialize buffers
  *retcode = ttd_fb_init(&fb1, g3_cli_args.block_size, infile1, 0);
  if (retcode < 0) {
    goto fb1_cleanup;
  }

  *retcode = ttd_fb_init(&fb2, g3_cli_args.block_size, infile2, 0);
  if (retcode < 0) {
    goto fb2_cleanup;
  }

  *retcode = ttd_fb_init(&fb3, g3_cli_args.block_size, infile3, 0);
  if (retcode < 0) {
    goto fb3_cleanup;
  }

  // Read out first records
  t1 = ttd_fb_pop(&fb1);
  t2 = ttd_fb_pop(&fb2);
  t3 = ttd_fb_pop(&fb3);

  // Loop over files while all three still have records
  while ((fb1.empty + fb2.empty + fb3.empty) == 0) {
    if (t1 <= t2) {
      if (t1 <= t3) { // t1 is lowest
        ttd_ccorr3_update(ccorr, 0, t1);
        t1 = ttd_fb_pop(&fb1);
      } else { // t3 is lowest
        ttd_ccorr3_update(ccorr, 2, t3);
        t3 = ttd_fb_pop(&fb3);
      }
    } else { // t2 < t1
      if (t2 <= t3) {
        ttd_ccorr3_update(ccorr, 1, t2);
        t2 = ttd_fb_pop(&fb2);
      } else {
        ttd_ccorr3_update(ccorr, 2, t3);
        t3 = ttd_fb_pop(&fb3);
      }
    }
  }

  // Clean up remaining pair of channels
  ttd_fb_t *fba, *fbb;
  size_t ia, ib;
  if (fb1.empty == 1) {
    fba = &fb2;
    fbb = &fb3;

    ta = t2;
    tb = t3;

    ia = 1;
    ib = 2;
  } else if (fb2.empty == 1) {
    fba = &fb1;
    fbb = &fb3;

    ta = t1;
    tb = t3;

    ia = 0;
    ib = 2;
  } else {
    fba = &fb1;
    fbb = &fb2;

    ta = t1;
    tb = t2;

    ia = 0;
    ib = 1;
  }

  while ((fba->empty == 0) && (fbb->empty == 0)) {
    if (ta <= tb) {
      ttd_ccorr3_update(ccorr, ia, ta);
      ta = ttd_fb_pop(fba);
    } else {
      ttd_ccorr3_update(ccorr, ib, tb);
      tb = ttd_fb_pop(fbb);
    }
  }

  while (fba->empty == 0) {
    ttd_ccorr3_update(ccorr, ia, ta);
    ta = ttd_fb_pop(fba);
  }

  while (fbb->empty == 0) {
    ttd_ccorr3_update(ccorr, ib, tb);
    tb = ttd_fb_pop(fbb);
  }

  fb3_cleanup:
  ttd_fb_cleanup(&fb3);

  fb2_cleanup:
  ttd_fb_cleanup(&fb2);

  fb1_cleanup:
  ttd_fb_cleanup(&fb1);

  return (ccorr);
}

int main(int argc, char *argv[]) {
  int retcode, exitcode = 0;

  retcode = g3_read_cli(argc, argv);

  if (retcode < 0) {
    exitcode = retcode;
    goto cleanup_g3_cli;
  } else if (retcode == G3_CLI_EXIT_RETCODE) {
    goto cleanup_g3_cli;
  }

  if (g3_cli_args.verbose) {
    g3_cli_print_options(G3_CLI_PRINTOPTIONS_NOVERBOSE);
  }

  char *infile1 = g3_cli_args.infile1;
  char *infile2 = g3_cli_args.infile2;
  char *infile3 = g3_cli_args.infile3;

  char *outfile = g3_cli_args.outfile;

  if (infile1 == NULL) {
    printf("Error: Missing input file 1. Please specify with the '-1' flag.\n");
    exitcode = -1;
    goto cleanup_g3_cli;
  }
  if (infile2 == NULL) {
    printf("Error: Missing input file 2. Please specify with the '-2' flag.\n");
    exitcode = -1;
    goto cleanup_g3_cli;
  }
  if (infile3 == NULL) {
    printf("Error: Missing input file 3. Please specify with the '-3' flag.\n");
    exitcode = -1;
    goto cleanup_g3_cli;
  }
  if (outfile == NULL) {
    printf("Error: Missing output file. Please specify with the '-o' flag.\n");
    exitcode = -1;
    goto cleanup_g3_cli;
  }

  ttd_ccorr3_t *g3_ccorr = ttd_g3(infile1, infile2, infile3, &retcode);

  ttd_ccorr3_write_csv(g3_ccorr, outfile);

  ttd_ccorr3_cleanup(g3_ccorr);
  free(g3_ccorr);

  cleanup_g3_cli:
  g3_cli_cleanup();

  exit(exitcode);
}
