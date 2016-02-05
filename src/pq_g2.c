//
// Created by Greg Steinbrecher on 1/31/16.
//

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pq_filebuffer.h>
#include <math.h>

#include "ttd_crosscorr2.h"
#include "pq_filebuffer.h"
#include "pq_g2_cli.h"
#include "pq_g2.h"
//#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/errno.h>

#define PROGRESS_BAR_ENABLE

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
                    pq_g2_cli_args.padded_window_time,
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
  ttd_t photonTime;
  int16_t chan, activeNum;
  retcode = 0;
  printf("hello\n");
#ifdef PROGRESS_BAR_ENABLE
  // Create shared memory
  int shmid = shmget(IPC_PRIVATE, 2*sizeof(int64_t), 0777|IPC_CREAT);
  printf("shmid: %d\n", shmid);
  printf("goodbye with error code %d\n", errno);
  int64_t *shvals = (int64_t *) shmat(shmid, 0, SHM_RND);


  shvals[0] = 0;
  shvals[1] = 0;

  /* Child process for drawing progress bar */
  if (fork()==0) {
    double progress, nextBarProgress;
    double stepBarProgress;
    double timeSoFar, timeRemaining;

    int64_t numBarSteps, totalBarSteps;
    struct timeb currentTime;
    double startTime;
    ftime(&currentTime);
    startTime = (currentTime.time + (double)(currentTime.millitm)/1000.0);

    nextBarProgress = 0.0;
    stepBarProgress = 1.0 / 40.0;

    numBarSteps = 0;
    totalBarSteps = (int64_t)round(1.0 / stepBarProgress) + 1;

    struct timespec waitTime, remTime;
    // Set waitTime to be 50ms
    waitTime.tv_nsec = 50000000;
    waitTime.tv_sec = 0;


    // Get shared memory
    shvals = (int64_t *) shmat(shmid, 0, SHM_RND);
    fprintf(stderr, "\n");

     // Use second value in array as done flag
    while (shvals[1] == 0) {
      nanosleep(&waitTime, &remTime);

      progress = ((double)shvals[0])/fb.file_info.num_records;
      // See if we need to draw more progress bar fills
      if (progress >= nextBarProgress) {
        // Update counter for progress bar (# of populated spaces)
        numBarSteps++;
        nextBarProgress += stepBarProgress;
      }

      // Draw progress bar
      fprintf(stderr, "\rProgress: [");
      for(i=0; i < numBarSteps; i++) {
        fprintf(stderr, "=");
      }
      fprintf(stderr, ">");
      for (i=0; i<(totalBarSteps - numBarSteps - 1); i++) {
        fprintf(stderr, " ");
      }
      fprintf(stderr, "] %3.0f%%", 100*progress);
      fflush(stderr);

      // Calculate predicted time remaining
      ftime(&currentTime);

      timeSoFar = (currentTime.time + (double)(currentTime.millitm)/1000.0) - startTime;
      timeRemaining = ((double)timeSoFar) * (1.0/progress - 1.0);
      fprintf(stderr, " (%4.02f seconds remaining)", timeRemaining);

    }
    fprintf(stderr, "\n");
    // Detach shared mem
    shmdt(shvals);
    exit(0);
  }
#endif



  // Loop over photons
  while ((fb.empty == 0)&&(retcode==0)) {
    activeNum = chanToActiveNum[chan];
    // Get next photon
    retcode = pq_fb_get_next(&fb, &photonTime, &chan);

    // Update the g2s
    for (i=0; i<num_active_channels-1; i++) {
      ttd_ccorr2_update(ccorrLookup[activeNum][i], ccorrNumLookup[activeNum][i], photonTime);
    }
#ifdef PROGRESS_BAR_ENABLE
    shvals[0] = fb.total_read;
#endif
  }
#ifdef PROGRESS_BAR_ENABLE
  shvals[1] = 1;
  // Wait for child process to exit
  wait(NULL);
  // Remove shared memory
  shmctl(shmid, IPC_RMID, NULL);
#endif
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
    ttd_ccorr2_write_csv(&ccorrs[i], outfile, pq_g2_cli_args.normalize, pq_g2_cli_args.int_time, pq_g2_cli_args.window_time);
  }

  // If all correlations have the same number of bins (they should, but hey, may as well check),
  // output a summed csv file

  int64_t num_bins;
  _Bool allSame = 1;
  num_bins = ccorrs[0].num_bins;
  for (i=1; i<nPairs; i++) {
    if (num_bins != ccorrs[i].num_bins) {allSame = 0;};
  }

  if (allSame) {
    // Hackish but works: add all the results into the first ccorr structure
    for (i=1; i<nPairs; i++) {
      // Needed in case we're normalizing
      // NOTE: have not actually checked that normalization still works here
      ccorrs[0].stats.rbs_counts[0] += ccorrs[i].stats.rbs_counts[0];
      ccorrs[0].stats.rbs_counts[1] += ccorrs[i].stats.rbs_counts[1];

      // Loop over bins to add to ccorrs[0].hist
      for (j=0; j<num_bins; j++) {
        ccorrs[0].hist[j] += ccorrs[i].hist[j];
      }
    }

    // Set output suffix to _sum.csv
    sprintf(outfile, "%s_sum.csv",
            outfile_prefix);

    // Write output
    ttd_ccorr2_write_csv(&ccorrs[0], outfile, pq_g2_cli_args.normalize, pq_g2_cli_args.int_time, pq_g2_cli_args.window_time);
  }


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
