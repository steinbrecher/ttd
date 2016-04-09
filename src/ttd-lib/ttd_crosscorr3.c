#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "ttd.h"
#include "ttd_ringbuffer.h"
#include "ttd_crosscorr3.h"

void ttd_ccorr3_init(ttd_ccorr3_t *ccorr, ttd_t bin_time, ttd_t window_time, size_t rb_size) {
  ccorr->bin_time = bin_time;
  ccorr->window_time = window_time;

  size_t num_bins = 1 + 2 * ttd_rounded_divide(window_time, bin_time);

  ccorr->num_bins = num_bins;
  ccorr->center_bin = (num_bins - 1)/2;
  ccorr->total_coinc = 0;

  ccorr->rbs_counts[0] = 0;
  ccorr->rbs_counts[1] = 0;
  ccorr->rbs_counts[2] = 0;

  ccorr->rbs[0] = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[0] = 1;

  ccorr->rbs[1] = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[1] = 1;

  ccorr->rbs[2] = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[2] = 1;

  ccorr->hist = (ttd_t *)calloc(sizeof(ttd_t *), num_bins * num_bins); 
  ccorr->hist_allocated = 1;
}

ttd_ccorr3_t *ttd_ccorr3_build(ttd_t bin_time, ttd_t window_time, size_t rb_size) {
  ttd_ccorr3_t *ccorr = (ttd_ccorr3_t *)malloc(sizeof(ttd_ccorr3_t));

  ttd_ccorr3_init(ccorr, bin_time, window_time, rb_size);
  return ccorr;
}

int other_rbs[3][2] = {{1, 2}, {0, 2}, {0, 1}};

// Note: Unlike in previous versions, rb_num is the channel the photon *did* arrive on
void ttd_ccorr3_update(ttd_ccorr3_t *ccorr, size_t rb_num, ttd_t time) {
  // Insert into the appropriate ringbuffer
  ttd_rb_insert(ccorr->rbs[rb_num], time);

  // Prune all the ringbuffers
  ttd_rb_prune(ccorr->rbs[0], time);
  ttd_rb_prune(ccorr->rbs[1], time);
  ttd_rb_prune(ccorr->rbs[2], time);

  int otherRbNum0, otherRbNum1;

  otherRbNum0 = other_rbs[rb_num][0];
  otherRbNum1 = other_rbs[rb_num][1];

  ttd_rb_t *other_rb1 = ccorr->rbs[otherRbNum0];
  ttd_rb_t *other_rb2 = ccorr->rbs[otherRbNum1];

  size_t  other_rb1_count = other_rb1->count;
  size_t other_rb2_count = other_rb2->count;

  size_t m, n;
  int64_t delta_bins1, delta_bins2;
  ttd_t times[3];

  times[rb_num] = time;
  if ((other_rb1_count > 0) && (other_rb2_count > 0)) {
    for (m=0; m < other_rb2_count; m++) {
      for (n=0; n < other_rb1_count; n++) {
	      times[otherRbNum0] = ttd_rb_get(other_rb1, n);
	      times[otherRbNum1] = ttd_rb_get(other_rb2, m);

	      // Column Delta
	      delta_bins1 = ((int64_t)ccorr->center_bin + int64_rounded_divide(times[1] - times[0], (int64_t)ccorr->bin_time));

	      // Row Delta
	      delta_bins2 = ((int64_t)ccorr->center_bin + int64_rounded_divide(times[2] - times[0], (int64_t)ccorr->bin_time));

        // tau1 is on the rows and tau2 is on the columns
        if ((delta_bins1 >= 0) && (delta_bins2 >= 0)) {
          ++ ccorr->hist[delta_bins2 + delta_bins1 * ccorr->num_bins];
          ++ ccorr->total_coinc;
        }
      }
    }
  }
}

void ttd_ccorr3_write_csv(ttd_ccorr3_t *ccorr, char *file_name) {
  FILE *output_file = fopen(file_name, "wb");
  ttd_t bin_time = ccorr->bin_time;
  ttd_t window_time = ccorr->window_time;
  size_t num_bins =  ccorr->num_bins;
  size_t m,n;
  int64_t dt1, dt2;
  // Histogram location is [num_bins * dbins1 + dbins2]
  // i.e. row index is the first delta time, column index the second

  for (m=0; m < num_bins; m++) { // Row Loop
    dt1 = -1 * (int64_t)(window_time + m*bin_time);
    for (n=0; n < num_bins; n++) { // Column loop
      dt2 = -1 * (int64_t)(window_time + n*bin_time);
      fprintf(output_file,
              "%" PRId64 ",%" PRId64 ",%" PRIu64 "\n",
              dt1,
              dt2,
              ccorr->hist[n + m*num_bins]);
    }
  }
  fclose(output_file);
}

void ttd_ccorr3_cleanup(ttd_ccorr3_t *ccorr) {
  ttd_rb_cleanup(ccorr->rbs[0]);
  ttd_rb_cleanup(ccorr->rbs[1]);
  ttd_rb_cleanup(ccorr->rbs[2]);

  if (ccorr->rbs_allocated[0]) {
    free(ccorr->rbs[0]);
    ccorr->rbs_allocated[0] = 0;
  }

  if (ccorr->rbs_allocated[1]) {
    free(ccorr->rbs[1]);
    ccorr->rbs_allocated[1] = 0;
  }

  if (ccorr->rbs_allocated[2]) {
    free(ccorr->rbs[2]);
    ccorr->rbs_allocated[2] = 0;
  }

  if (ccorr->hist_allocated) {
    free(ccorr->hist);
  }
}
