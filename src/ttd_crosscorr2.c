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
#include "ttd_ringbuffer.h"
#include "ttd_crosscorr2.h"

void ttd_ccorr2_init(ttd_ccorr2_t *ccorr, ttd_t bin_time, ttd_t window_time, ttd_rb_t *rb1, ttd_rb_t *rb2) {
  ccorr->bin_time = bin_time;
  ccorr->window_time = window_time;

  int num_bins = (int)(2*ttd_rounded_divide(window_time, bin_time) + 1);
  ccorr->num_bins = num_bins;
  ccorr->center_bin = (num_bins - 1)/2;

  ccorr->stats.rbs_counts[0] = 0;
  ccorr->stats.rbs_counts[1] = 0;
  
  ccorr->rbs[0] = rb1;
  ccorr->rbs[1] = rb2;
  ccorr->hist = (ttd_t *)calloc(sizeof(ttd_t), num_bins);
  ccorr->hist_allocated = 1;
}

ttd_ccorr2_t *ttd_ccorr2_build(ttd_t bin_time, ttd_t window_time, int rb_size) {
  ttd_ccorr2_t *ccorr = (ttd_ccorr2_t *)malloc(sizeof(ttd_ccorr2_t));

  ttd_rb_t *rb1 = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[0] = 1;

  ttd_rb_t *rb2 = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[1] = 1;

  ttd_ccorr2_init(ccorr, bin_time, window_time, rb1, rb2);
  return ccorr;
}

// Note: Unlike in previous versions, rb_num is the channel the photon *did* arrive on
void ttd_ccorr2_update(ttd_ccorr2_t *ccorr, int rb_num, ttd_t time) {
  // Insert into the appropriate ringbuffer
  ttd_rb_insert(ccorr->rbs[rb_num], time);

  // Prune both ringbuffers
  ttd_rb_prune(ccorr->rbs[0], time);
  ttd_rb_prune(ccorr->rbs[1], time);

  // Sign is 1 if rb_num is 1, -1 if it's 0 (i.e. delta_t = rb2_time - rb1_time)
  int other_rb_num = 1-rb_num;
  ttd_rb_t *other_rb = ccorr->rbs[other_rb_num];
  int other_rb_count = other_rb->count;
  
  int64_t times[2];
  int n, delta_bins;
  times[rb_num] = time;
  if (other_rb->count > 0) {
    for (n=0; n < other_rb_count; n++) {
      times[other_rb_num] = ttd_rb_get(other_rb, n);
      delta_bins = (int)(ccorr->center_bin + int64_rounded_divide(times[1]-times[0], ccorr->bin_time));
      ++ ccorr->hist[delta_bins];
    }
  }
}

void ttd_ccorr2_write_csv(ttd_ccorr2_t *ccorr, char *file_name) {
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

void ttd_ccorr2_cleanup(ttd_ccorr2_t *ccorr) {
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





