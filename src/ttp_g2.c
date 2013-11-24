#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ttp.h"
#include "ttp_cli.h"

#include "ttd.h"
#include "ttd_ringbuffer.h"
#include "ttd_crosscorr.h"
#include "ttd_filebuffer.h"

#define PHOTONBLOCK 16384

int ttp_g2() {
  int64_t output_buffer_count = 0;
  uint64_t t1, t2;

  ttd_ccorr_t *ccorr = ttd_ccorr_build(1024, ttp_cli_args.window_time);

  ttd_fb_t fb1, fb2;

  // Initialize buffers
  ttd_fb_init(&fb1, ttp_cli_args.block_size, ttp_cli_args.infile1);
  ttd_fb_init(&fb2, ttp_cli_args.block_size, ttp_cli_args.infile2);

  while((fb1.empty == 0) && (fb2.empty == 0)) {
    if (t1 <= t2) {
      ttd_ccorr_update(ccorr, 0, t1);
      t1 = ttd_fb_pop(&fb1);
    }
    else {
      ttd_ccorr_update(ccorr, 1, t2);
      t2 = ttd_fb_pop(&fb2);
      output_buffer_count ++;
    }
  }

  while (fb1.empty == 0) {
    ttd_ccorr_update(ccorr, 0, t1);
    t1 = ttd_fb_pop(&fb1);
  }

  while (fb2.empty == 0) {
    ttd_ccorr_update(ccorr, 1, t1);
    t2 = ttd_fb_pop(&fb2);
    output_buffer_count ++;
  }

  ttd_ccorr_write_csv(ccorr, ttp_cli_args.outfile1);

  ttd_ccorr_cleanup(ccorr);
  ttd_fb_cleanup(&fb1);
  ttd_fb_cleanup(&fb2);

  return(0);
}


int main(int argc, char* argv[]) {
  ttp_read_cli(argc, argv);

  ttp_g2();

  exit(0);
}
