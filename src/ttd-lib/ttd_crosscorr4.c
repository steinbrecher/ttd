#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "ttd.h"
#include "ttd_ringbuffer.h"
#include "ttd_crosscorr4.h"

void ttd_ccorr4_init(ttd_ccorr4_t *ccorr, ttd_t bin_time, ttd_t window_time, size_t rb_size) {
  ccorr->bin_time = bin_time;
  ccorr->window_time = window_time;

  size_t num_bins = 1 + 2 * ttd_rounded_divide(window_time, bin_time);

  ccorr->num_bins = num_bins;
  ccorr->center_bin = (num_bins - 1) / 2;

  ccorr->total_coinc = 0;

  ccorr->rbs_counts[0] = 0;
  ccorr->rbs_counts[1] = 0;
  ccorr->rbs_counts[2] = 0;
  ccorr->rbs_counts[3] = 0;

  ttd_rb_t *rb1 = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[0] = 1;

  ttd_rb_t *rb2 = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[1] = 1;

  ttd_rb_t *rb3 = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[2] = 1;

  ttd_rb_t *rb4 = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[3] = 1;

  ccorr->rbs[0] = rb1;
  ccorr->rbs[1] = rb2;
  ccorr->rbs[2] = rb3;
  ccorr->rbs[3] = rb4;

  ccorr->hist = (ttd_t *) calloc(sizeof(ttd_t *), num_bins * num_bins * num_bins);
  ccorr->hist_allocated = 1;
}

ttd_ccorr4_t *ttd_ccorr4_build(ttd_t bin_time, ttd_t window_time, size_t rb_size) {
  ttd_ccorr4_t *ccorr = (ttd_ccorr4_t *) malloc(sizeof(ttd_ccorr4_t));

  ttd_ccorr4_init(ccorr, bin_time, window_time, rb_size);
  return ccorr;
}

int ccorr4_other_rbs[4][3] = {{1, 2, 3},
                              {0, 2, 3},
                              {0, 1, 3},
                              {0, 1, 2}};

// Note: Unlike in previous versions, rb_num is the channel the photon *did* arrive on
void ttd_ccorr4_update(ttd_ccorr4_t *ccorr, size_t rb_num, ttd_t time) {
  // Insert into the appropriate ringbuffer
  ttd_rb_insert(ccorr->rbs[rb_num], time);

  // Prune all the ringbuffers
  ttd_rb_prune(ccorr->rbs[0], time);
  ttd_rb_prune(ccorr->rbs[1], time);
  ttd_rb_prune(ccorr->rbs[2], time);
  ttd_rb_prune(ccorr->rbs[3], time);

  // Collect references to other three ringbuffers
  ttd_rb_t *other_rb1 = ccorr->rbs[ccorr4_other_rbs[rb_num][0]];
  ttd_rb_t *other_rb2 = ccorr->rbs[ccorr4_other_rbs[rb_num][1]];
  ttd_rb_t *other_rb3 = ccorr->rbs[ccorr4_other_rbs[rb_num][2]];

  size_t other_rb1_count = other_rb1->count;
  size_t other_rb2_count = other_rb2->count;
  size_t other_rb3_count = other_rb3->count;

  // Check if we have a correlation event
  if ((other_rb1_count > 0) && (other_rb2_count > 0) && (other_rb3_count > 0)) {
    int64_t delta_t1, delta_t2, delta_t3;
    size_t num_bins = ccorr->num_bins;
    size_t m, n, k;
    int delta_bins1, delta_bins2, delta_bins3;
    ttd_t times[4];
    ttd_t bin_time = ccorr->bin_time;

    times[rb_num] = time;

    for (m = 0; m < other_rb3_count; m++) {
      for (n = 0; n < other_rb2_count; n++) {
        for (k = 0; k < other_rb1_count; k++) {
          // Write various times to the proper slots in the array
          times[ccorr4_other_rbs[rb_num][0]] = ttd_rb_get(other_rb1, k);
          times[ccorr4_other_rbs[rb_num][1]] = ttd_rb_get(other_rb2, n);
          times[ccorr4_other_rbs[rb_num][2]] = ttd_rb_get(other_rb2, m);

          // Calculate tau1 bin
          delta_t1 = times[1] - times[0];
          delta_bins1 = (int) (ccorr->center_bin + int64_rounded_divide(delta_t1, bin_time));

          // Calculate tau2 bin
          delta_t2 = times[2] - times[0];
          delta_bins2 = (int) (ccorr->center_bin + int64_rounded_divide(delta_t2, bin_time));

          // Calculate tau3 bin
          delta_t3 = times[3] - times[0];
          delta_bins3 = (int) (ccorr->center_bin + int64_rounded_divide(delta_t3, bin_time));

          // tau1 is along the rows, tau2 is along the columns, tau3 is the matrix index
          if ((delta_bins1 >= 0) && (delta_bins2 >= 0) && (delta_bins3 >= 0)) {
            ++ccorr->hist[delta_bins2 + num_bins * delta_bins1 + num_bins * num_bins * delta_bins3];
            ++ccorr->total_coinc;
          }
        }
      }
    }
  }
}

void ttd_ccorr4_update_no_insert(ttd_ccorr4_t *ccorr, size_t rb_num, ttd_t time) {
  // Collect references to other three ringbuffers
  ttd_rb_t *other_rb1 = ccorr->rbs[ccorr4_other_rbs[rb_num][0]];
  ttd_rb_t *other_rb2 = ccorr->rbs[ccorr4_other_rbs[rb_num][1]];
  ttd_rb_t *other_rb3 = ccorr->rbs[ccorr4_other_rbs[rb_num][2]];

  size_t other_rb1_count = other_rb1->count;
  size_t other_rb2_count = other_rb2->count;
  size_t other_rb3_count = other_rb3->count;

  // Check if we have a correlation event
  if ((other_rb1_count > 0) && (other_rb2_count > 0) && (other_rb3_count > 0)) {
    int64_t delta_t1, delta_t2, delta_t3;
    size_t num_bins = ccorr->num_bins;
    size_t m, n, k;
    int delta_bins1, delta_bins2, delta_bins3;
    ttd_t times[4];
    ttd_t bin_time = ccorr->bin_time;

    times[rb_num] = time;

    for (m = 0; m < other_rb3_count; m++) {
      for (n = 0; n < other_rb2_count; n++) {
        for (k = 0; k < other_rb1_count; k++) {
          // Write various times to the proper slots in the array
          times[ccorr4_other_rbs[rb_num][0]] = ttd_rb_get(other_rb1, k);
          times[ccorr4_other_rbs[rb_num][1]] = ttd_rb_get(other_rb2, n);
          times[ccorr4_other_rbs[rb_num][2]] = ttd_rb_get(other_rb2, m);

          // Calculate tau1 bin
          delta_t1 = times[1] - times[0];
          delta_bins1 = (int) (ccorr->center_bin + int64_rounded_divide(delta_t1, bin_time));

          // Calculate tau2 bin
          delta_t2 = times[2] - times[0];
          delta_bins2 = (int) (ccorr->center_bin + int64_rounded_divide(delta_t2, bin_time));

          // Calculate tau3 bin
          delta_t3 = times[3] - times[0];
          delta_bins3 = (int) (ccorr->center_bin + int64_rounded_divide(delta_t3, bin_time));

          // tau1 is along the rows, tau2 is along the columns, tau3 is the matrix index
          if ((delta_bins1 >= 0) && (delta_bins2 >= 0) && (delta_bins3 >= 0)) {
            ++ccorr->hist[delta_bins2 + num_bins * delta_bins1 + num_bins * num_bins * delta_bins3];
            ++ccorr->total_coinc;
          }
        }
      }
    }
  }
}

/* TODO: Pull this into a utils file
// Note: Has side-effect of a malloc
char* append_before_extension(char* to_append, char* old_filename) {
  size_t i;
  size_t old_len = strlen(old_filename);
  size_t app_len = strlen(to_append);
  size_t new_len = old_len + app_len + 1;
  _Bool has_period = 0;
  size_t period_index=0;
  char *new_str = (char *)malloc(new_len*sizeof(char));

  // Try to find last period
  for (i=old_len-1; i>=0; i--) {
    if (old_filename[i] == '.') {
      has_period = 1;
      period_index = i;
      break;
    }
  }
  
  // If no period found, just copy over the two pieces and return
  if (has_period == 0) {
    strncpy(new_str, old_filename, old_len);
    strncpy(new_str+old_len, to_append, app_len);
    return(new_str);
  }
  
  // Otherwise, copy over the first string in two pieces with append in middle
  strncpy(new_str, old_filename, period_index);
  strncpy(new_str+period_index, to_append, app_len);
  strncpy(new_str+period_index+app_len, old_filename+period_index, old_len-period_index);
  return new_str;
}
 */

void ttd_ccorr4_write_csv(ttd_ccorr4_t *ccorr, char *file_name) {
  FILE *output_file = fopen(file_name, "wb");
  ttd_t bin_time = ccorr->bin_time;
  ttd_t window_time = ccorr->window_time;
  size_t num_bins = ccorr->num_bins;
  size_t m, n, o;
  int64_t dt1, dt2, dt3;
  // Histogram location is [num_bins * dbins1 + dbins2]
  // i.e. row index is the first delta time, column index the second
  for (m = 0; m < num_bins; m++) { // dt1 Loop
    dt1 = -1 * (int64_t) (window_time + m * bin_time);
    for (n = 0; n < num_bins; n++) { // dt2 loop
      dt2 = -1 * (int64_t) (window_time + n * bin_time);
      for (o = 0; o < num_bins; o++) { // dt3 loop
        dt3 = -1 * (int64_t) (window_time + o * bin_time);
        fprintf(output_file,
                "%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRIu64 "\n",
                dt1,
                dt2,
                dt3,
                ccorr->hist[n + m * num_bins + o * num_bins * num_bins]);
      }
    }
  }
  fclose(output_file);
}

void ttd_ccorr4_cleanup(ttd_ccorr4_t *ccorr) {
  int i;
  for (i = 0; i < 4; i++) {
    ttd_rb_cleanup(ccorr->rbs[i]);
    if (ccorr->rbs_allocated[i]) {
      free(ccorr->rbs[i]);
      ccorr->rbs_allocated[i] = 0;
    }
  }

  if (ccorr->hist_allocated) {
    free(ccorr->hist);
  }
}
