//
// Created by Greg Steinbrecher on 1/31/16.
//

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ttd.h"
#include "ttd_ringbuffer.h"
#include "ttd_crosscorr2.h"
#include "ttd_filebuffer.h"
#include "pq_filebuffer.h"
#include "pq_g2_cli.h"
#include "pq_g2.h"

int pq_g2_many(char* infile, char* outfile_prefix) {
  int retcode=0;

  // Make file buffer
  pq_fb_t fb;
  pq_fb_init(&fb, infile);

  // Set active channels
  int16_t i,j;
  int64_t channel_offsets[PQ_HH_MAX_CHANNELS];
  int64_t min_offset = 0;
  for (i=0; i<PQ_HH_MAX_CHANNELS; i++) {
    if (pq_g2_cli_args.channel_active[i]) {
      //pq_fb_enable_channel(&fb, i);
      channel_offsets[i] = pq_g2_cli_args.channel_offset[i];
      if (channel_offsets[i] < min_offset) {
        min_offset = channel_offsets[i];
      }
    }
    else {
      //pq_fb_disable_channel(&fb, i);
      channel_offsets[i] = 0;
    }
    //printf("Offset channel %d by %d\n", i, channel_offsets[i]);
  }

  // Make all offsets non-negative, and set filebuffer offsets
  for (i=0; i<PQ_HH_MAX_CHANNELS; i++) {
    fb.channel_offsets[i] = channel_offsets[i] + (-1 * min_offset);
  }

  // Allocate and initialize the correlation structures
  int16_t nPairs = (fb.num_active_channels * (fb.num_active_channels-1)) / 2;
  ttd_ccorr2_t ccorrs[nPairs];
  for (i=0; i<nPairs; i++) {
    ttd_ccorr2_init(ccorrs + i,
                    pq_g2_cli_args.bin_time,
                    pq_g2_cli_args.window_time,
                    pq_g2_cli_args.rb_size);
//    ccorrs[i] = ttd_ccorr2_build(pq_g2_cli_args.bin_time,
//                                 pq_g2_cli_args.window_time,
//                                 pq_g2_cli_args.rb_size);
  }

  // Select the pairings for cross-correlation
  int16_t num_active_channels = fb.num_active_channels;
  int16_t ccorr_pairs[nPairs][2];
  int16_t count;
  int16_t chanToActiveNum[PQ_HH_MAX_CHANNELS];



  // Reverse lookup for active num
  for(i=0; i<num_active_channels; i++) {
    chanToActiveNum[fb.active_channels[i]] = i;
  }

  count = 0;
  for (i=0; i<num_active_channels-1; i++) {
    for (j=i+1; j<num_active_channels; j++) {
      ccorr_pairs[count][0] = i;
      ccorr_pairs[count][1] = j;
      count++;
    }
  }

  // Precompute correlation lookups
  // Each of the N channels is a member of N-1 ccorr structures
  ttd_ccorr2_t *ccorrLookup[num_active_channels][num_active_channels-1];
  int16_t ccorrNumLookup[num_active_channels][num_active_channels-1];


  for (i=0; i<num_active_channels; i++) {
    count = 0;
    for (j=0; j<nPairs; j++) {
      if (ccorr_pairs[j][0] == i) {
        ccorrLookup[i][count] = &ccorrs[j];
        ccorrNumLookup[i][count] = 0;
        count++;
      }
      else if (ccorr_pairs[j][1] == i) {
        ccorrLookup[i][count] = &ccorrs[j];
        ccorrNumLookup[i][count] = 1;
        count++;
      }
    }
  }

  // Grab first photon
  ttd_t time;
  int16_t chan, activeNum;
  retcode = pq_fb_get_next(&fb, &time, &chan);


  // Loop over photons
  while ((fb.empty == 0)&&(retcode==0)) {
    activeNum = chanToActiveNum[chan];
    for (i=0; i<num_active_channels-1; i++) {
      ttd_ccorr2_update(ccorrLookup[activeNum][i], ccorrNumLookup[activeNum][i], time);
    }

    retcode = pq_fb_get_next(&fb, &time, &chan);
  }

  // Output the files
  char outfile[strlen(outfile_prefix)+20];
  for (i=0; i<nPairs; i++) {
    if(ccorrs[i].total_coinc == 0) {
      printf("WARNING: Correlation between channels %d and %d had no counts.\n", ccorr_pairs[i][0], ccorr_pairs[i][1]);
      continue;
    }
    sprintf(outfile, "%s_%d-%d.csv",
            outfile_prefix,
            ccorr_pairs[i][0],
            ccorr_pairs[i][1]);
    ttd_ccorr2_write_csv(&ccorrs[i], outfile, pq_g2_cli_args.normalize, pq_g2_cli_args.int_time);
  }

//  for (chan=0; chan<PQ_HH_MAX_CHANNELS; chan++) {
//    if (fb.num_read_per_channel[chan] > 0) {
//      printf("Number of records on channel %d: %lu\n", chan, fb.num_read_per_channel[chan]);
//    }
//  }

  // Clean up correlations
    for (i=0; i<nPairs; i++) {
      ttd_ccorr2_cleanup(&ccorrs[i]);
      //free(ccorrs[i]);
    }

  // Clean up file buffer
  pq_fb_cleanup(&fb);

  return retcode;
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
  char *outfile_prefix = pq_g2_cli_args.outfile_prefix;

  if (outfile_prefix == NULL) {
    printf("Error: Missing output file prefix. Please specify with the '-o' flag.\n");
    exitcode = -1;
    goto cleanup_pq_g2_cli;
  }

  pq_g2_many(pq_g2_cli_args.infile, outfile_prefix);

  cleanup_pq_g2_cli:
  pq_g2_cli_cleanup();

  exit_block:
  exit(exitcode);
}
