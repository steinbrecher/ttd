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
#include "g3_cli.h"
#include "ttd_ringbuffer.h"
#include "ttd_crosscorr3.h"

void ttd_ccorr3_init(ttd_ccorr3_t *ccorr, ttd_rb_t *rb1, ttd_rb_t *rb2, ttd_rb_t *rb3) {
  ttd_t bin_time = g3_cli_args.bin_time;
  ttd_t window_time = g3_cli_args.window_time;

  ccorr->bin_time = bin_time;
  ccorr->window_time = window_time;

  int num_bins = (int)(2*ttd_rounded_divide(window_time, bin_time) + 1);

  ccorr->num_bins = num_bins;
  ccorr->center_bin = (num_bins - 1)/2;

  ccorr->stats.rbs_counts[0] = 0;
  ccorr->stats.rbs_counts[1] = 0;
  ccorr->stats.rbs_counts[2] = 0;
  
  ccorr->rbs[0] = rb1;
  ccorr->rbs[1] = rb2;
  ccorr->rbs[2] = rb3;

  ccorr->hist = (ttd_t *)calloc(sizeof(ttd_t *), num_bins * num_bins); 
  ccorr->hist_allocated = 1;
}

ttd_ccorr3_t *ttd_ccorr3_build(int rb_size, ttd_t rb_duration) {
  ttd_ccorr3_t *ccorr = (ttd_ccorr3_t *)malloc(sizeof(ttd_ccorr3_t));

  ttd_rb_t *rb1 = ttd_rb_build(rb_size, rb_duration);
  ccorr->rbs_allocated[0] = 1;

  ttd_rb_t *rb2 = ttd_rb_build(rb_size, rb_duration);
  ccorr->rbs_allocated[1] = 1;

  ttd_rb_t *rb3 = ttd_rb_build(rb_size, rb_duration);
  ccorr->rbs_allocated[2] = 1;

  ttd_ccorr3_init(ccorr, rb1, rb2, rb3);
  return ccorr;
}

int other_rbs[3][2] = {{1, 2}, {0, 2}, {0, 1}};

// Note: Unlike in previous versions, rb_num is the channel the photon *did* arrive on
void ttd_ccorr3_update(ttd_ccorr3_t *ccorr, int rb_num, ttd_t time) {
  // Insert into the appropriate ringbuffer
  ttd_rb_insert(ccorr->rbs[rb_num], time);

  // Prune all the ringbuffers
  ttd_rb_prune(ccorr->rbs[0], time);
  ttd_rb_prune(ccorr->rbs[1], time);
  ttd_rb_prune(ccorr->rbs[2], time);

  ttd_rb_t *other_rb1 = ccorr->rbs[other_rbs[rb_num][0]];
  ttd_rb_t *other_rb2 = ccorr->rbs[other_rbs[rb_num][1]];

  int other_rb1_count = other_rb1->count;
  int other_rb2_count = other_rb2->count;

  int64_t delta_t1, delta_t2;
  int m, n, delta_bins1, delta_bins2;
  ttd_t times[3];

  times[rb_num] = time;
  if ((other_rb1_count > 0) && (other_rb2_count > 0)) {
    for (m=0; m < other_rb2_count; m++) {
      for (n=0; n < other_rb1_count; n++) {
	times[other_rbs[rb_num][0]] = ttd_rb_get(other_rb1, n);
	times[other_rbs[rb_num][1]] = ttd_rb_get(other_rb2, m);

	// Column Delta
	delta_t1 = times[1] - times[0];
	delta_bins1 = (int)(ccorr->center_bin + int64_rounded_divide(delta_t1, ccorr->bin_time));

	// Row Delta
	delta_t2 = times[2] - times[0];
	delta_bins2 = (int)(ccorr->center_bin + int64_rounded_divide(delta_t2, ccorr->bin_time));

	// tau1 is on the rows and tau2 is on the columns
	++ ccorr->hist[delta_bins2 + ccorr->num_bins*delta_bins1];
      }
    }
  }
}

void ttd_ccorr3_write_csv(ttd_ccorr3_t *ccorr, char *file_name) {
  FILE *output_file = fopen(file_name, "wb");
  ttd_t bin_time = ccorr->bin_time;
  ttd_t window_time = ccorr->window_time;
  int num_bins =  ccorr->num_bins;
  int m,n;

  for (m=0; m < num_bins; m++) { // Row Loop
    for (n=0; n < num_bins-1; n++) { // Column loop
      fprintf(output_file, "%" PRIu64 ", ", ccorr->hist[n + m*num_bins]);
    }
    // Pull this out to avoid the trailing comma
    fprintf(output_file, "%" PRIu64 "\n", ccorr->hist[num_bins - 1 + m*num_bins]);
  }
  fclose(output_file);
}

// Note: Has side-effect of a malloc
char* append_before_extension(char* to_append, char* old_filename) {
  int i, old_len = strlen(old_filename), app_len=strlen(to_append);
  int new_len = old_len + app_len + 1;
  int has_period = 0, period_index;
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

void ttd_ccorr3_write_times_csv(ttd_ccorr3_t *ccorr, char *file_name) {
  FILE *output_file = fopen(file_name, "wb");
  ttd_t bin_time = ccorr->bin_time;
  ttd_t window_time = ccorr->window_time;
  int num_bins = ccorr->num_bins;
  int n;
  int64_t time;
  for (n=0; n<num_bins; n++) {
    time = -1*(int64_t)window_time + n*(int64_t)bin_time;
    fprintf(output_file, "%" PRId64 "\n", time);
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
