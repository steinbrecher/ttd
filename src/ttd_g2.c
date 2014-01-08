#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ttp.h"
#include "ttd_g2_cli.h"

#include "ttd.h"
#include "ttd_ringbuffer.h"
#include "ttd_crosscorr2.h"
#include "ttd_filebuffer.h"

// Note: Make sure to clean up and then free the ccorr object generated by this function
ttd_ccorr2_t *ttd_g2(char* infile1, char* infile2, int *retcode) {
  int64_t output_buffer_count = 0;
  uint64_t t1, t2;

  ttd_ccorr2_t *ccorr = ttd_ccorr2_build(ttd_g2_cli_args.bin_time, ttd_g2_cli_args.window_time, 1024);

  ttd_fb_t fb1, fb2;

  int64_t offset2 = ttd_g2_cli_args.infile2_offset;

  // Initialize buffers
  *retcode = ttd_fb_init(&fb1, ttd_g2_cli_args.block_size, infile1, 0);
  if (retcode < 0) 
    goto fb1_cleanup;

  *retcode = ttd_fb_init(&fb2, ttd_g2_cli_args.block_size, infile2, offset2);
  if (retcode < 0)
    goto fb2_cleanup;

  // Read out first records
  t1 = ttd_fb_pop(&fb1);
  t2 = ttd_fb_pop(&fb2);

  while((fb1.empty == 0) && (fb2.empty == 0)) {
    if (t1 <= t2) {
      ttd_ccorr2_update(ccorr, 0, t1);
      t1 = ttd_fb_pop(&fb1);
    }
    else {
      ttd_ccorr2_update(ccorr, 1, t2);
      t2 = ttd_fb_pop(&fb2);
      output_buffer_count ++;
    }
  }

  while (fb1.empty == 0) {
    ttd_ccorr2_update(ccorr, 0, t1);
    t1 = ttd_fb_pop(&fb1);
  }

  while (fb2.empty == 0) {
    ttd_ccorr2_update(ccorr, 1, t2);
    t2 = ttd_fb_pop(&fb2);
    output_buffer_count ++;
  }

 fb2_cleanup:
  ttd_fb_cleanup(&fb2);

 fb1_cleanup:
  ttd_fb_cleanup(&fb1);

  return(ccorr);
}


int main(int argc, char* argv[]) {
  int retcode, exitcode=0;

  retcode = ttp_read_cli(argc, argv);

  if (retcode < 0) {
    exitcode = retcode;
    goto cleanup_ttd_g2_cli;
  }
  else if (retcode == TTD_G2_CLI_EXIT_RETCODE) {
    goto cleanup_ttd_g2_cli;
  }

  if (ttd_g2_cli_args.verbose) {
    ttp_print_options(TTP_PRINTOPTIONS_NOVERBOSE);
  }

  char *infile1 = ttd_g2_cli_args.infile1;
  char *infile2 = ttd_g2_cli_args.infile2;
  char *outfile = ttd_g2_cli_args.outfile;
  
  if (infile1 == NULL) {
    printf("Error: Missing input file 1. Please specify with the '-i' flag.\n");
    exitcode = -1;
    goto cleanup_ttd_g2_cli;
  }
  if (infile2 == NULL) {
    printf("Error: Missing input file 2. Please specify with the '-I' flag.\n");
    exitcode = -1;
    goto cleanup_ttd_g2_cli;
  }
  if (outfile == NULL) {
    printf("Error: Missing output file. Please specify with the '-o' flag.\n");
    exitcode = -1;
    goto cleanup_ttd_g2_cli;
  }

  ttd_ccorr2_t *g2_ccorr = ttd_g2(infile1, infile2, &retcode);

  ttd_ccorr2_write_csv(g2_ccorr, outfile);


 cleanup_ccorr:
  ttd_ccorr2_cleanup(g2_ccorr);
  free(g2_ccorr);

 cleanup_ttd_g2_cli:
  ttd_g2_cli_cleanup();
  
 exit_block:
  exit(exitcode);
}
