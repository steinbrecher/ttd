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
#include "ttd_crosscorr3.h"

void ttd_ccorr3_init(ttd_ccorr3_t *ccorr, ttd_rb_t *rb1, ttd_rb_t *rb2, ttd_rb_t *rb3) {
  ttd_t bin_time = ttp_cli_args.bin_time;
  ttd_t window_time = ttp_cli_args.window_time;

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
  ccorr->rbs[1] = rb3;

  ccorr->hist = (ttd_t *)calloc(num_bins * num_bins * sizeof(ttd_t *)); 
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

int sign_lookup[3][2] = {{-1, -1}, {1, -1}, {1, 1}};
int other_rbs[3][2] = {{1, 2}, {0, 2}, {0, 1}};

// Note: Unlike in previous versions, rb_num is the channel the photon *did* arrive on
void ttd_ccorr3_update(ttd_ccorr3_t *ccorr, int rb_num, ttd_t time) {
  // Insert into the appropriate ringbuffer
  ttd_rb_insert(ccorr->rbs[rb_num], time);

  // Prune all the ringbuffers
  ttd_rb_prune(ccorr->rbs[0], time);
  ttd_rb_prune(ccorr->rbs[1], time);
  ttd_rb_prune(ccorr->rbs[2], time);

  int sign1 = sign_lookup[rb_num][0];
  int sign2 = sign_lookup[rb_num][1];

  ttd_rb_t *other_rb1 = ccorr->rbs[other_rbs[rb_num][0]];
  ttd_rb_t *other_rb2 = ccorr->rbs[other_rbs[rb_num][1]];

  int other_rb1_count = other_rb1->count;
  int other_rb2_count = other_rb2->count;

  ttd_t delta_t1, delta_t2;
  int m, n, delta_bins1, delta_bins2;
  
  if ((other_rb1_count > 0) && (other_rb2_count > 0)) {
    for (m=0; m < other_rb1_count; m++) {
      for (n=0; n < other_rb2_count; n++) {
	// Column Delta
	delta_t1 = time - ttd_rb_get(other_rb1, n);
	delta_bins1 = (int)(ccorr->center_bin + sign*ttd_rounded_divide(delta_t1, ccorr->bin_time));

	// Row Delta
	delta_t2 = time - ttd_rb_get(other_rb2, m);
	delta_bins2 = (int)(ccorr->center_bin + sign*ttd_rounded_divide(delta_t2, ccorr->bin_time));
	
	++ ccorr->hist[delta_bins1 + ccorr->num_bins*delta_bins2];
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
