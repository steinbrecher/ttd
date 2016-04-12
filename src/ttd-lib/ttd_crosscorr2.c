#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "ttd.h"
#include "ttd_crosscorr2.h"

// TODO: Error checking / return value for ttd_ccorr2_init
/** \brief Initializes ttd_ccorr2_t structs
 *
 * Summary:
 *  - Allocate rbs[0] and rbs[1] (setting ccorr->rbs_allocated[] = {1,1})
 *  - Assign ccorr->bin_time and ccorr->window_time according to function arguments
 *  - Calculate number of bins needed and allocate histogram (setting ccorr->hist_allocated = 1)
 *  - Initialize the rest of the variables
 *
 * Allocations:
 *  - ccorr->rbs[0] (w/ all side effects of initializing a ttd_rb_t struct)
 *  - ccorr->rbs[1] (w/ all side effects of initializing a ttd_rb_t struct)
 *  - ccorr->hist (of size ccorr->num_bins; see function for that calculation)
 * */
void ttd_ccorr2_init(ttd_ccorr2_t *ccorr, ttd_t bin_time, ttd_t window_time, size_t rb_size) {
  // TODO: Error checking here
  ccorr->rbs[0] = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[0] = 1;

  // TODO: Error checking here
  ccorr->rbs[1] = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[1] = 1;

  ccorr->bin_time = bin_time;
  ccorr->window_time = window_time;
  ccorr->total_coinc = 0;

  size_t num_bins = (2*ttd_rounded_divide(ccorr->window_time, ccorr->bin_time) + 1);
  ccorr->num_bins = num_bins;
  ccorr->center_bin = (num_bins - 1)/2;

  ccorr->rbs_counts[0] = 0;
  ccorr->rbs_counts[1] = 0;

  ccorr->hist = (ttd_t *)calloc(num_bins, sizeof(ttd_t));
  if (ccorr->hist != NULL) {
    ccorr->hist_allocated = 1;
  }
  else {
    printf("Histogram not allocated!\n");
    ccorr->hist_allocated = 0;
  }
}

/** /brief Helper function to allocate and initialize a ccorr2 struct
 *
 *
 *
 * */
 ttd_ccorr2_t *ttd_ccorr2_build(ttd_t bin_time, ttd_t window_time, size_t rb_size) {
  ttd_ccorr2_t *ccorr = (ttd_ccorr2_t *)calloc(1, sizeof(ttd_ccorr2_t));
  ttd_ccorr2_init(ccorr, bin_time, window_time, rb_size);
  return ccorr;
}

// Per-photon-arrival function that updates the correlation histogram and prunes the ringbuffers
// rb_num is the channel the photon arrived on
void ttd_ccorr2_update(ttd_ccorr2_t *ccorr, size_t rb_num, ttd_t time) {
  // Insert into the appropriate ringbuffer
  ttd_rb_insert(ccorr->rbs[rb_num], time);

  // Increment totals counter
  ccorr->rbs_counts[rb_num]++;

  // Prune both ringbuffers
  ttd_rb_prune(ccorr->rbs[0], time);
  ttd_rb_prune(ccorr->rbs[1], time);

  // Sign is 1 if rb_num is 1, -1 if it's 0 (i.e. delta_t = rb2_time - rb1_time)
  size_t other_rb_num = 1 - rb_num;
  ttd_rb_t *other_rb = ccorr->rbs[other_rb_num];
  size_t other_rb_count = other_rb->count;


  int64_t times[2], delta_bins;
  int n;
  size_t start, size;
  times[rb_num] = (int64_t)time;

  if (other_rb->count > 0) {
    start = other_rb->start;
    size = other_rb->size;
    for (n=0; n < other_rb_count; n++) {
      //times[other_rb_num] = ttd_rb_get(other_rb, n);
      times[other_rb_num] = other_rb->times[(start + n) % size];
      delta_bins = ((int64_t)ccorr->center_bin +
                    int64_rounded_divide(times[1]-times[0], (int64_t)ccorr->bin_time));
      if (delta_bins >= 0) {
        ++ ccorr->hist[delta_bins];
        ++ ccorr->total_coinc;
      }
    }
  }
}

void ttd_ccorr2_update_no_insert(ttd_ccorr2_t *ccorr, size_t rb_num, ttd_t time) {
  // Increment the local totals counter (could be globalized also)
  ccorr->rbs_counts[rb_num]++;

  // Sign is 1 if rb_num is 1, -1 if it's 0 (i.e. delta_t = rb2_time - rb1_time)
  size_t other_rb_num = 1 - rb_num;
  ttd_rb_t *other_rb = ccorr->rbs[other_rb_num];
  size_t other_rb_count = other_rb->count;


  int64_t times[2], delta_bins;
  int n;
  size_t start, size;
  times[rb_num] = (int64_t)time;

  if (other_rb->count > 0) {
    start = other_rb->start;
    size = other_rb->size;
    for (n=0; n < other_rb_count; n++) {
      //times[other_rb_num] = ttd_rb_get(other_rb, n);
      times[other_rb_num] = other_rb->times[(start + n) % size];
      delta_bins = ((int64_t)ccorr->center_bin +
                    int64_rounded_divide(times[1]-times[0], (int64_t)ccorr->bin_time));
      if (delta_bins >= 0) {
        ++ ccorr->hist[delta_bins];
        ++ ccorr->total_coinc;
      }
    }
  }
}

void ttd_ccorr2_write_csv(ttd_ccorr2_t *ccorr, char *file_name, int normalize, ttd_t int_time, ttd_t write_window) {
  FILE *output_file = fopen(file_name, "wb");
  int64_t window_time = ccorr->window_time;
  int m;

  /* Compute and print out the normalization stats */
  double rate0, rate1, rate_product;
  ttd_t bin_time = ccorr->bin_time;
  uint64_t int_time_ps = 1000000000000 * (ttd_t)int_time;

  // If we don't have an explicit integration time, infer it from last seen photon
  if (int_time == 0) {
    ttd_t last_time1, last_time2;
    last_time1 = ttd_rb_get(ccorr->rbs[0], 0);
    last_time2 = ttd_rb_get(ccorr->rbs[1], 0);
    if (last_time1 > last_time2) {
      int_time_ps = last_time1;
    }
    else {
      int_time_ps = last_time2;
    }
  }

  uint64_t num_bins = int_time_ps / bin_time;
  //printf("\nIntegration Time: %" PRIu64" \n", int_time_ps);
  //printf("Number of Time Bins: %" PRIu64 " \n", num_bins);
  // Calculate rates
  rate0 = ((double)ccorr->rbs_counts[0]) / (double)num_bins;
  rate1 = ((double)ccorr->rbs_counts[1]) / (double)num_bins;
  rate_product = rate0 * rate1;
  //printf("Channel 1 Counts: %" PRIu64 " \n", ccorr->stats.rbs_counts[0]);
  //printf("Channel 2 Counts: %" PRIu64 " \n", ccorr->stats.rbs_counts[1]);

  // NOTE: Strangely, this causes valgrind to report memorly leaks on OSX
  // Seems to be an issue with valgrind, not anything here
  //printf("Rate Product: %f \n", rate_product);
  int64_t bin_offset;
  if (normalize == 0) {
    for (m=0; m < ccorr->num_bins; m++) {
      bin_offset = (m*bin_time) - window_time;
      // Don't include bins outside window time
      if (llabs(bin_offset) > write_window) {
        //fprintf(stderr, "Skipping bin offset %"PRId64"\n", bin_offset);
        continue;
      }
      fprintf(output_file, "%" PRId64", %" PRIu64 " \n",
              bin_offset, (ccorr->hist[m]));
    }
  }
  else {
    // Perform normalization and output to file

    double norm_denom, norm_counts;
    for (m=0; m < ccorr->num_bins; m++) {
      bin_offset = (m*bin_time) - window_time;
      // Don't include bins outside window time
      if (llabs(bin_offset) > write_window) {
        //fprintf(stderr, "Skipping bin offset %"PRIu64"\n", bin_offset);
        continue;
      }
      norm_denom = rate_product * (double)(num_bins - llabs(bin_offset));
      norm_counts = ((double)ccorr->hist[m]) / norm_denom;
      fprintf(output_file, "%" PRId64", %f \n", bin_offset, norm_counts);
    }
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





