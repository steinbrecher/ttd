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
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <sys/timeb.h>
#include <locale.h>
#include <ttd_crosscorr3.h>

#include "ttd_crosscorr2.h"
#include "ttd_crosscorr3.h"
#include "pq_filebuffer.h"
#include "pq_g2_cli.h"
#include "pq_g2.h"

#define PROGRESS_BAR_ENABLE

#ifdef PROGRESS_BAR_ENABLE
void draw_progress_bar(int shmid, int64_t num_records) {
  struct timeb currentTime;
  double startTime, runTime;

  ftime(&currentTime);
  startTime = (currentTime.time + (double)(currentTime.millitm)/1000.0);

  int64_t i, lastVal;
  int64_t *shvals;
  double progress, nextBarProgress;
  double stepBarProgress;
  double timeSoFar, timeRemaining;
  int64_t nFails = 0;

  int64_t numBarSteps, totalBarSteps;

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
  if (shvals == NULL) {
    perror("shmat");
    exit(1);
  }

  fprintf(stderr, CURSOROFF "");

  // Use second value in array as done flag
  while (shvals[1] == 0) {
    nanosleep(&waitTime, &remTime);
    lastVal = shvals[0];
    progress = ((double)shvals[0])/num_records;
    // Check for stalled parent
    if (shvals[0] == 0) {
      nFails++;
      if (nFails >= 5) {
        fprintf(stderr, KNRM KCYN KRED "\nERROR: No progress is being made. Exiting...\n" KNRM CURSORON);
        // Detach shared mem
        shmdt(shvals);
        shmctl(shmid, IPC_RMID, NULL);
        perror("parent died");
        exit(1);
      }
    }
    else {
      nFails = 0;
      //lastVal = shvals[0];
    }
    // See if we need to draw more progress bar fills
    if (progress >= nextBarProgress) {
      // Update counter for progress bar (# of populated spaces)
      numBarSteps++;
      nextBarProgress += stepBarProgress;
    }

    // Draw progress bar
    fprintf(stderr, CURSOROFF "\r");
    fprintf(stderr, KNRM KCYN KRED "[");
    for(i=0; i < numBarSteps; i++) {
      fprintf(stderr, "=");
    }
    fprintf(stderr, ">");
    for (i=0; i<(totalBarSteps - numBarSteps - 1); i++) {
      fprintf(stderr, " ");
    }
    fprintf(stderr, "] %3.0f%% Done", 100*progress);
    fflush(stderr);

    // Calculate predicted time remaining
    ftime(&currentTime);

    timeSoFar = (currentTime.time + (double)(currentTime.millitm)/1000.0) - startTime;
    timeRemaining = ((double)timeSoFar) * (1.0/progress - 1.0);
    fprintf(stderr, " (%4.02f seconds remaining) " KNRM CURSORON, timeRemaining);

  }
  fprintf(stderr, "\r" KNRM KBGRN KBLD "[");
  for(i=0; i < totalBarSteps; i++) {
    fprintf(stderr, "=");
  }
  fprintf(stderr, ">] %3.0f%% Done", 100.0);
  fprintf(stderr, " (%4.02f seconds remaining)", 0.0);
  fprintf(stderr, "\n" KNRM);
  fprintf(stderr, CURSORON "");
  // Detach shared mem
  shmdt(shvals);
  //
  exit(0);

}
#endif //PROGRESS_BAR_ENABLED

int pq_g2_many(char* infile, char* outfile_prefix) {
  int retcode=0;

  // Make file buffer
  pq_fb_t fb;
  pq_fb_init(&fb, infile);

  // Set active channels
  size_t i,j,k;
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
  }

  // Make all offsets non-negative, and set filebuffer offsets
  for (i=0; i<PQ_HH_MAX_CHANNELS; i++) {
    fb.channel_offsets[i] = (ttd_t)(channel_offsets[i] + (-1 * min_offset));
  }

  size_t num_active = fb.num_active_channels;

  // num_pairs = Choose[num_active, 2]
  size_t num_pairs = num_active * (num_active - 1);
  num_pairs /= 2;

  // Allocate and initialize the g2 correlation structures
  ttd_ccorr2_t g2_ccorrs[num_pairs];
  for (i=0; i<num_pairs; i++) {
    ttd_ccorr2_init(g2_ccorrs + i,
                    pq_g2_cli_args.bin_time,
                    pq_g2_cli_args.padded_window_time,
                    pq_g2_cli_args.rb_size);
  }


  // num_triplets = Choose[num_active, 3]
  size_t num_triplets = num_active * (num_active - 1) * (num_active - 2);
  num_triplets /= 6;

  // Allocate and initialize the g3 correlation structures
  ttd_ccorr3_t g3_ccorrs[num_triplets];
  for (i=0; i<num_triplets; i++) {
    ttd_ccorr3_init(g3_ccorrs + i,
                    pq_g2_cli_args.bin_time,
                    pq_g2_cli_args.window_time,
                    pq_g2_cli_args.rb_size);
  }

  // Back up which channels are active
  size_t active_channels[PQ_HH_MAX_CHANNELS];
  memcpy(active_channels, fb.active_channels, PQ_HH_MAX_CHANNELS * sizeof(size_t));

  // Reverse lookup for active num
  size_t chanToActiveNum[PQ_HH_MAX_CHANNELS];
  for(i=0; i<num_active; i++) {
    chanToActiveNum[fb.active_channels[i]] = i;
  }

  // Select the pairings for g2 cross-correlation
  size_t ccorr_pairs[num_pairs][2];
  size_t count;
  count = 0;
  for (i=0; i<num_active-1; i++) {
    for (j=i+1; j<num_active; j++) {
      ccorr_pairs[count][0] = i;
      ccorr_pairs[count][1] = j;
      count++;
    }
  }

  // Select the pairings for g3 cross-correlation
  size_t ccorr_triplets[num_triplets][3];
  count = 0;
  for (i=0; i<num_active-2; i++) {
    for (j=i+1; j<num_active-1; j++) {
      for (k=j+1; k<num_active; k++) {
        ccorr_triplets[count][0] = i;
        ccorr_triplets[count][1] = j;
        ccorr_triplets[count][2] = k;
        count++;
      }
    }
  }

  // Precompute g2 correlation lookups
  // Each of the N channels is a member of N-1 ccorr structures
  size_t num_g2_friends = num_active-1;
  ttd_ccorr2_t *g2_ccorr_lookup[num_active][num_g2_friends];
  size_t g2_ccorr_num_lookup[num_active][num_g2_friends];

  for (i=0; i<num_active; i++) {
    count = 0;
    for (j=0; j<num_pairs; j++) {
      if (ccorr_pairs[j][0] == i) {
        g2_ccorr_lookup[i][count] = &g2_ccorrs[j];
        g2_ccorr_num_lookup[i][count] = 0;
        count++;
      }
      else if (ccorr_pairs[j][1] == i) {
        g2_ccorr_lookup[i][count] = &g2_ccorrs[j];
        g2_ccorr_num_lookup[i][count] = 1;
        count++;
      }
    }
  }

  // Precompute g3 correlation lookups
  // Each of the N channels is a member of Choose[N-1,2] g3_ccorr structures
  size_t num_g3_friends = (num_active-1)*(num_active-2);
  num_g3_friends /= 2;
  ttd_ccorr3_t *g3_ccorr_lookup[num_active][num_g3_friends];
  size_t g3_ccorr_num_lookup[num_active][num_g3_friends];

  for (i=0; i<num_active; i++) {
    count = 0;
    for (j=0; j<num_triplets; j++) {
      if (ccorr_triplets[j][0] == i) {
        g3_ccorr_lookup[i][count] = &g3_ccorrs[j];
        g3_ccorr_num_lookup[i][count] = 0;
        count++;
      }
      else if (ccorr_triplets[j][1] == i) {
        g3_ccorr_lookup[i][count] = &g3_ccorrs[j];
        g3_ccorr_num_lookup[i][count] = 1;
        count++;
      }
      else if (ccorr_triplets[j][2] == i) {
        g3_ccorr_lookup[i][count] = &g3_ccorrs[j];
        g3_ccorr_num_lookup[i][count] = 2;
        count++;
      }
    }
  }

  // Set up timer structure
  struct timeb currentTime;
  double startTime, runTime;

  ftime(&currentTime);
  startTime = (currentTime.time + (double)(currentTime.millitm)/1000.0);

#ifdef PROGRESS_BAR_ENABLE
  // Get shared memory
  int shmid;
  shmid = shmget(IPC_PRIVATE, 2*sizeof(int64_t), 0777|IPC_CREAT|IPC_EXCL);
  if (shmid < 0) {
    perror("shmget");
    exit(1);
  }

  // Initialize shared memory
  int64_t *shvals = (int64_t *) shmat(shmid, 0, SHM_RND);
  shvals[0] = 0;
  shvals[1] = 0;

  /* Child process for drawing progress bar */
  if (fork()==0) {
    draw_progress_bar(shmid, fb.file_info.num_records);

  }
#endif // PROGRESS_BAR_ENABLE

  // Loop over photons
  ttd_t photonTime=0;
  size_t chan=0, activeNum=0;
  retcode = 0;
  ttd_ccorr2_t *g2_ccorr_ptr;
  ttd_ccorr3_t *g3_ccorr_ptr;
  size_t rbNum;

  while ((fb.empty == 0)&&(retcode==0)) {
    // Get next photon
    retcode = pq_fb_get_next(&fb, &photonTime, &chan);
    activeNum = chanToActiveNum[chan];

    // Update the g2s
    for (i=0; i<num_g2_friends; i++) {
      // Note: do not inline ccorrPtr into the ttd_ccorr2_update call
      // Results in null pointer being passed under certain situations
      g2_ccorr_ptr = g2_ccorr_lookup[activeNum][i];
      rbNum = g2_ccorr_num_lookup[activeNum][i];
      ttd_ccorr2_update(g2_ccorr_ptr, rbNum, photonTime);
    }

    // Update the g3s
    for (i=0; i<num_g3_friends; i++) {
      g3_ccorr_ptr = g3_ccorr_lookup[activeNum][i];
      rbNum = g3_ccorr_num_lookup[activeNum][i];
      ttd_ccorr3_update(g3_ccorr_ptr, rbNum, photonTime);
    }

#ifdef PROGRESS_BAR_ENABLE
    shvals[0] = fb.total_read;
#endif

  }

  ftime(&currentTime);
  runTime = (currentTime.time + (double)(currentTime.millitm)/1000.0) - startTime;

#ifdef PROGRESS_BAR_ENABLE
  shvals[1] = 1;
  // Wait for child process to exit
  wait(NULL);
  // Remove shared memory
  shmctl(shmid, IPC_RMID, NULL);
#endif

  double intTime = ceil((double)photonTime / 1e12);
  int64_t totalCounts=0, total_g2_coinc=0, total_g3_coinc=0;

  // Print statistics
  fprintf(stderr, KHEAD1 "\nCounts per channel:\n");
  size_t numSeen;
  for (i=0; i<num_active; i++) {
    chan = active_channels[i];
    numSeen = fb.num_read_per_channel[chan];
    fprintf(stderr,
            KHEAD2 KCYN "  %lu: " KNUMBER "%'" PRId64 " " KRATE "(%'0.01f Hz)\n",
            chan, (int64_t)numSeen, (double)numSeen / intTime);
    totalCounts += numSeen;
  }

  fprintf(stderr, KHEAD1 "\ng(2) Coincidences:\n");
  for (i=0; i<num_pairs; i++) {
    fprintf(stderr,
            KHEAD2 KCYN "  %lu-%lu: " KNUMBER "%'" PRIu64 " " KRATE "(%'0.01f Hz)\n",
            active_channels[ccorr_pairs[i][0]],
            active_channels[ccorr_pairs[i][1]],
            g2_ccorrs[i].total_coinc,
            (double)g2_ccorrs[i].total_coinc/intTime);
    total_g2_coinc += g2_ccorrs[i].total_coinc;
  }

  fprintf(stderr, KHEAD1 "\ng(3) Coincidences:\n");
  for (i=0; i<num_triplets; i++) {
    fprintf(stderr,
            KHEAD2 KCYN "  %lu-%lu-%lu: " KNUMBER "%'" PRIu64 " " KRATE "(%'0.01f Hz)\n",
            active_channels[ccorr_triplets[i][0]],
            active_channels[ccorr_triplets[i][1]],
            active_channels[ccorr_triplets[i][2]],
            g3_ccorrs[i].total_coinc,
            (double)g3_ccorrs[i].total_coinc/intTime);
    total_g3_coinc += g3_ccorrs[i].total_coinc;
  }

  fprintf(stderr,
          KHEAD1 "\ng(2) Summary" KHEAD2 ": Processed " KNUMBER "%'" PRId64 KHEAD2 " coincidences between "
                  KNUMBER "%'" PRId64 KHEAD2 " photons " KHEAD2 "in " KTIME "%0.02f seconds "KNRM"\n",
          total_g2_coinc, totalCounts, runTime);
  fprintf(stderr,
          KHEAD1 "g(3) Summary" KHEAD2 ": Processed " KNUMBER "%'" PRId64 KHEAD2 " coincidences between "
                  KNUMBER "%'" PRId64 KHEAD2 " photons " KHEAD2 "in " KTIME "%0.02f seconds "KNRM"\n",
          total_g3_coinc, totalCounts, runTime);

  // Output the g2 files
  fprintf(stderr, "\n");
  char outfile[strlen(outfile_prefix)+20];
  for (i=0; i<num_pairs; i++) {
    if(g2_ccorrs[i].total_coinc == 0) {
      fprintf(stderr, KHEAD1 "WARNING" KHEAD2 ": Correlation between channels %lu and %lu had no coincidences.\n" KNRM,
              active_channels[ccorr_pairs[i][0]], active_channels[ccorr_pairs[i][1]]);
      continue;
    }
    sprintf(outfile, "%s_%lu-%lu.csv",
            outfile_prefix,
            active_channels[ccorr_pairs[i][0]],
            active_channels[ccorr_pairs[i][1]]);
    ttd_ccorr2_write_csv(&g2_ccorrs[i], outfile, pq_g2_cli_args.normalize, pq_g2_cli_args.int_time, pq_g2_cli_args.window_time);
  }

  // Output the g3 files
  char times_outfile[strlen(outfile_prefix)+20];
  for (i=0; i<num_triplets; i++) {
    if(g3_ccorrs[i].total_coinc == 0) {
      fprintf(stderr, KHEAD1 "WARNING" KHEAD2 ": Correlation between channels %lu and %lu had no coincidences.\n" KNRM,
              active_channels[ccorr_pairs[i][0]], active_channels[ccorr_pairs[i][1]]);
      continue;
    }
    sprintf(outfile, "%s_%lu-%lu-%lu.csv",
            outfile_prefix,
            active_channels[ccorr_triplets[i][0]],
            active_channels[ccorr_triplets[i][1]],
            active_channels[ccorr_triplets[i][2]]);
    ttd_ccorr3_write_csv(&g3_ccorrs[i], outfile);
    if (i==0) {
      sprintf(times_outfile, "%s_g3times.csv", outfile_prefix);
      ttd_ccorr3_write_times_csv(&g3_ccorrs[0], times_outfile);
    }

  }



  // If all g2 correlations have the same number of bins (they should, but hey, may as well check),
  // output a summed csv file
  int64_t num_bins;
  _Bool allSame = 1;
  num_bins = g2_ccorrs[0].num_bins;
  for (i=1; i<num_pairs; i++) {
    if (num_bins != g2_ccorrs[i].num_bins) {allSame = 0;};
  }

  if (allSame) {
    // Hackish but works: add all the results into the first ccorr structure
    for (i=1; i<num_pairs; i++) {
      // Needed in case we're normalizing
      // NOTE: have not actually checked that normalization still works here
      g2_ccorrs[0].rbs_counts[0] += g2_ccorrs[i].rbs_counts[0];
      g2_ccorrs[0].rbs_counts[1] += g2_ccorrs[i].rbs_counts[1];

      // Loop over bins to add to g2_ccorrs[0].hist
      for (j=0; j<num_bins; j++) {
        g2_ccorrs[0].hist[j] += g2_ccorrs[i].hist[j];
      }
    }

    // Set output suffix to _sum.csv
    sprintf(outfile, "%s_g2sum.csv",
            outfile_prefix);

    // Write output
    ttd_ccorr2_write_csv(&g2_ccorrs[0], outfile, pq_g2_cli_args.normalize, pq_g2_cli_args.int_time, pq_g2_cli_args.window_time);
  }

  // If all g3 correlations have the same number of bins (they should, but hey, may as well check),
  // output a summed csv file
  allSame = 1;
  num_bins = g3_ccorrs[0].num_bins;
  for (i=1; i<num_triplets; i++) {
    if (num_bins != g3_ccorrs[i].num_bins) {allSame = 0;};
  }

  if (allSame) {
    // Hackish but works: add all the results into the first ccorr structure
    for (i=1; i<num_triplets; i++) {
      // Loop over bins to add to g2_ccorrs[0].hist
      for (j=0; j<num_bins*num_bins; j++) {
          g3_ccorrs[0].hist[j] += g3_ccorrs[i].hist[j];
      }
    }

    // Set output suffix to _sum.csv
    sprintf(outfile, "%s_g3sum.csv",
            outfile_prefix);

    // Write output
    ttd_ccorr3_write_csv(&g3_ccorrs[0], outfile);
  }



  // Clean up g2 correlations
  for (i=0; i<num_pairs; i++) {
    ttd_ccorr2_cleanup(&g2_ccorrs[i]);
  }

  // Clean up g3 correlations
  for (i=0; i<num_triplets; i++) {
    ttd_ccorr3_cleanup(&g3_ccorrs[i]);
  }

  // Clean up file buffer
  pq_fb_cleanup(&fb);

  return retcode;
}


int main(int argc, char* argv[]) {
  int retcode, exitcode=0;

  fprintf(stderr, "\n\t\t" KHEAD1 "Time Tagged Data Processor" KNRM ": g(2) Cross-Correlation\n\n" KNRM);

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
  fprintf(stderr, "\n");

  exit(exitcode);
}
