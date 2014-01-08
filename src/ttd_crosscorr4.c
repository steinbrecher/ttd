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
#include "ttd_crosscorr4.h"

void ttd_ccorr4_init(ttd_ccorr4_t *ccorr, ttd_t bin_time, ttd_t window_time, 
		     ttd_rb_t *rb1, ttd_rb_t *rb2, ttd_rb_t *rb3, ttd_rb_t *rb4) {
  ccorr->bin_time = bin_time;
  ccorr->window_time = window_time;

  int num_bins = (int)(2*ttd_rounded_divide(window_time, bin_time) + 1);

  ccorr->num_bins = num_bins;
  ccorr->center_bin = (num_bins - 1)/2;

  ccorr->stats.rbs_counts[0] = 0;
  ccorr->stats.rbs_counts[1] = 0;
  ccorr->stats.rbs_counts[2] = 0;
  ccorr->stats.rbs_counts[3] = 0;
  
  ccorr->rbs[0] = rb1;
  ccorr->rbs[1] = rb2;
  ccorr->rbs[2] = rb3;
  ccorr->rbs[3] = rb4;

  ccorr->hist = (ttd_t *)calloc(num_bins * num_bins * num_bins, sizeof(ttd_t *)); 
  ccorr->hist_allocated = 1;
}

ttd_ccorr4_t *ttd_ccorr4_build(ttd_t bin_time, ttd_t window_time, int rb_size) {
  ttd_ccorr4_t *ccorr = (ttd_ccorr4_t *)malloc(sizeof(ttd_ccorr4_t));

  ttd_rb_t *rb1 = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[0] = 1;

  ttd_rb_t *rb2 = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[1] = 1;

  ttd_rb_t *rb3 = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[2] = 1;

  ttd_rb_t *rb4 = ttd_rb_build(rb_size, window_time);
  ccorr->rbs_allocated[3] = 1;

  ttd_ccorr4_init(ccorr, bin_time, window_time, rb1, rb2, rb3, rb4);
  return ccorr;
}

int ccorr4_other_rbs[4][3] = {{1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2}};

// Note: Unlike in previous versions, rb_num is the channel the photon *did* arrive on
void ttd_ccorr4_update(ttd_ccorr4_t *ccorr, int rb_num, ttd_t time) {
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

  int other_rb1_count = other_rb1->count;
  int other_rb2_count = other_rb2->count;
  int other_rb3_count = other_rb3->count;

  // Check if we have a correlation event
  if ((other_rb1_count > 0) && (other_rb2_count > 0) && (other_rb3_count > 0)) {
    int64_t delta_t1, delta_t2, delta_t3, num_bins = ccorr->num_bins;
    int m, n, k, delta_bins1, delta_bins2, delta_bins3;
    ttd_t times[4];
    ttd_t bin_time = ccorr->bin_time;

    times[rb_num] = time;

    for (m=0; m < other_rb3_count; m++) {
      for (n=0; n < other_rb2_count; n++) {
	for (k=0; k < other_rb1_count; k++) {
	  // Write various times to the proper slots in the array
	  times[ccorr4_other_rbs[rb_num][0]] = ttd_rb_get(other_rb1, k);
	  times[ccorr4_other_rbs[rb_num][1]] = ttd_rb_get(other_rb2, n);
	  times[ccorr4_other_rbs[rb_num][2]] = ttd_rb_get(other_rb2, m);

	  // Calculate tau1 bin
	  delta_t1 = times[1] - times[0];
	  delta_bins1 = (int)(ccorr->center_bin + ttd_rounded_divide(delta_t1, bin_time));

	  // Calculate tau2 bin
	  delta_t2 = times[2] - times[0];
	  delta_bins2 = (int)(ccorr->center_bin + ttd_rounded_divide(delta_t2, bin_time));

	  // Calculate tau3 bin
	  delta_t3 = times[3] - times[0];
	  delta_bins3 = (int)(ccorr->center_bin + ttd_rounded_divide(delta_t3, bin_time));

	  // tau1 is along the rows, tau2 is along the columns, tau3 is the matrix index
	  ++ ccorr->hist[delta_bins2 + num_bins*delta_bins1 + num_bins*num_bins*delta_bins3];
	}
      }
    }
  }
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

void ttd_ccorr4_write_csv(ttd_ccorr4_t *ccorr, char *file_name) {
  FILE *output_file;
  char *appended_fname;
  char appended[20]; 

  ttd_t bin_time = ccorr->bin_time;
  ttd_t window_time = ccorr->window_time;
  int num_bins =  ccorr->num_bins;
  int64_t matrix_offset, row_offset;
  int m,n,k,column;
  // We are going to iterate over each 2-by-2, outputting to a different CSV file
  for (k=0; k<num_bins; k++) {
    sprintf(appended, "-%d", k);
    appended_fname = append_before_extension(appended, file_name);
    output_file = fopen(appended_fname, "wb");
    printf("Outputting file: %s\n", appended_fname);
    free(appended_fname);
    
    matrix_offset = k*num_bins*num_bins;
    for (m=0; m < num_bins; m++) { // Row Loop
      row_offset = m*num_bins;
      for (column=0; column < num_bins-1; column++) { // Column loop
	fprintf(output_file, "%" PRIu64 ", ", ccorr->hist[column + row_offset + matrix_offset]);
      }
      // Pull this out to avoid the trailing comma
      fprintf(output_file, "%" PRIu64 "\n", ccorr->hist[num_bins - 1 + row_offset + matrix_offset]);
    }
    fclose(output_file);
  }
}


void ttd_ccorr4_write_times_csv(ttd_ccorr4_t *ccorr, char *file_name) {
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

void ttd_ccorr4_cleanup(ttd_ccorr4_t *ccorr) {
  int i;
  for (i=0; i<4; i++) {
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
