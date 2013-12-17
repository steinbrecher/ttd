#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ttd.h"
#include "ttp_cli.h"
#include "ttd_ringbuffer.h"
#include "ttd_crosscorr.h"

void ttd_ccorr_init(ttd_ccorr_t *ccorr, ttd_rb_t *rb1, ttd_rb_t *rb2) {
  ttd_t bin_time = ttp_cli_args.bin_time;
  ttd_t window_time = ttp_cli_args.window_time;

  ccorr->bin_time = bin_time;
  ccorr->window_time = window_time;

  int num_bins = (int)(2*ttd_rounded_divide(window_time, bin_time) + 1);
  ccorr->num_bins = num_bins;
  ccorr->center_bin = (num_bins - 1)/2;

  ccorr->stats.rbs_counts[0] = 0;
  ccorr->stats.rbs_counts[1] = 0;
  
  ccorr->rbs[0] = rb1;
  ccorr->rbs[1] = rb2;
  ccorr->hist = (ttd_t *)calloc(num_bins, sizeof(ttd_t));
  ccorr->hist_allocated = 1;
}

ttd_ccorr_t *ttd_ccorr_build(int rb_size, ttd_t rb_duration) {
  ttd_ccorr_t *ccorr = (ttd_ccorr_t *)malloc(sizeof(ttd_ccorr_t));

  ttd_rb_t *rb1 = ttd_rb_build(rb_size, rb_duration);
  ccorr->rbs_allocated[0] = 1;

  ttd_rb_t *rb2 = ttd_rb_build(rb_size, rb_duration);
  ccorr->rbs_allocated[1] = 1;

  ttd_ccorr_init(ccorr, rb1, rb2);
  return ccorr;
}

// Note: Unlike in previous versions, rb_num is the channel the photon *did* arrive on
void ttd_ccorr_update(ttd_ccorr_t *ccorr, int rb_num, ttd_t time) {
  // Insert into the appropriate ringbuffer
  ttd_rb_insert(ccorr->rbs[rb_num], time);

  // Prune both ringbuffers
  ttd_rb_prune(ccorr->rbs[0], time);
  ttd_rb_prune(ccorr->rbs[1], time);

  // Sign is 1 if rb_num is 1, -1 if it's 0 (i.e. delta_t = rb2_time - rb1_time)
  int sign = 2*rb_num - 1;
  ttd_rb_t *other_rb = ccorr->rbs[1-rb_num];
  int other_rb_count = other_rb->count;
  //  printf("%d\n", other_rb_count);
  
  ttd_t delta_t;
  int n, delta_bins;
  
  if (other_rb->count > 0) {
    for (n=0; n < other_rb_count; n++) {
      delta_t = time - ttd_rb_get(other_rb, n);
      delta_bins = (int)(ccorr->center_bin + sign*ttd_rounded_divide(delta_t, ccorr->bin_time));
      ++ ccorr->hist[delta_bins];
    }
  }
}

void ttd_ccorr_write_csv(ttd_ccorr_t *ccorr, char *file_name) {
  FILE *output_file = fopen(file_name, "wb");
  ttd_t bin_time = ccorr->bin_time;
  ttd_t window_time = ccorr->window_time;
  int m;
  for (m=0; m < ccorr->num_bins; m++) {
    fprintf(output_file, "%" PRId64", %" PRIu64 "\n", 
	    ((m*bin_time) - window_time), (ccorr->hist[m]));
  }

  fclose(output_file);
}

void ttd_ccorr_cleanup(ttd_ccorr_t *ccorr) {
  ttd_rb_cleanup(ccorr->rbs[0]);
  ttd_rb_cleanup(ccorr->rbs[1]);
  if (ccorr->rbs_allocated[0]) {
    free(ccorr->rbs[0]);
    ccorr->rbs_allocated[0] = 0;
  }
  if (ccorr->rbs_allocated[1]) {
    free(ccorr->rbs[1]);
    ccorr->rbs_allocated[1] = 0;
  }
  if (ccorr->hist_allocated) {
    free(ccorr->hist);
  }
}





