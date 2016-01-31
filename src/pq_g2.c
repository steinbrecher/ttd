//
// Created by Greg Steinbrecher on 1/31/16.
//

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <stdlib.h>

#include "ttd.h"
#include "ttd_ringbuffer.h"
#include "ttd_crosscorr2.h"
#include "ttd_filebuffer.h"
#include "pq_filebuffer.h"
#include "pq_g2_cli.h"
#include "pq_g2.h"

ttd_ccorr2_t *pq_g2(char* infile, int *retcode, int chan1, int chan2) {
  int64_t output_buffer_count = 0;
  int retcode_here=0;
  ttd_ccorr2_t *ccorr = ttd_ccorr2_build(pq_g2_cli_args.bin_time,
                                         pq_g2_cli_args.window_time,
                                         pq_g2_cli_args.rb_size);

  if (ccorr->hist_allocated == 0) {
    printf("ERROR: Histogram not allocated");
    exit(-1);
  }

  // Make filebuffer
  pq_fb_t fb;
  if (pq_fb_init(&fb, infile) < 0) {
    goto fb_cleanup;
  }

  ttd_t time;
  int16_t chan;
  int8_t channel_map[8] = {0,0,0,0,0,0,0,0};
  channel_map[chan1] = 1;
  channel_map[chan2] = 2;

  retcode_here = pq_fb_pop(&fb, &time, &chan);
  while ((fb.empty == 0)&&(retcode_here==0)) {
    switch(chan) {
      case 1:
        ttd_ccorr2_update(ccorr, 0, time);
        break;
      case 2:
        ttd_ccorr2_update(ccorr, 1, time);
        break;
      default:
        break;
    }
    retcode_here = pq_fb_pop(&fb, &time, &chan);
  }

  fb_cleanup:
  pq_fb_cleanup(&fb);
  *retcode = retcode_here;
  return(ccorr);
}


int main(int argc, char* argv[]) {
  int retcode, exitcode=0;

  retcode = pq_g2_read_cli(argc, argv);

  if (retcode < 0) {
    exitcode = retcode;
    goto cleanup_pq_g2_cli;
  }
  else if (retcode == PQ_G2_CLI_EXIT_RETCODE) {
    goto cleanup_pq_g2_cli;
  }

  if (pq_g2_cli_args.verbose) {
    pq_g2_print_options(PQ_G2_PRINTOPTIONS_NOVERBOSE);
  }
  char *outfile = pq_g2_cli_args.outfile;

  if (outfile == NULL) {
    printf("Error: Missing output file. Please specify with the '-o' flag.\n");
    exitcode = -1;
    goto cleanup_pq_g2_cli;
  }

  ttd_ccorr2_t *g2_ccorr = pq_g2(pq_g2_cli_args.infile, &retcode, pq_g2_cli_args.channel1, pq_g2_cli_args.channel2);

  ttd_ccorr2_write_csv(g2_ccorr, outfile, pq_g2_cli_args.normalize, pq_g2_cli_args.int_time);


  cleanup_ccorr:
  ttd_ccorr2_cleanup(g2_ccorr);
  free(g2_ccorr);

  cleanup_pq_g2_cli:
  pq_g2_cli_cleanup();

  exit_block:
  exit(exitcode);
}
