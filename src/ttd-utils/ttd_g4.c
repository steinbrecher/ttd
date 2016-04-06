#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ttd_g4_cli.h"

#include "ttd.h"
#include "ttd_ringbuffer.h"
#include "ttd_crosscorr4.h"
#include "ttd_filebuffer.h"

ttd_ccorr4_t *ttd_g4(char *infile1, char *infile2, char *infile3, char *infile4, int *retcode) {

  int64_t output_buffer_count = 0;
  //uint64_t t1, t2, t3, t4, ta, tb, tc, count;
  uint64_t count, times[4], times_alt[4];
  int i, j, k; //counters

  ttd_ccorr4_t *ccorr = ttd_ccorr4_build(g4_cli_args.bin_time, g4_cli_args.window_time, 1024);

  ttd_fb_t fbs[4];
  //  ttd_fb_t fb1, fb2, fb3;

  // Initialize buffers
  *retcode = ttd_fb_init(&fbs[0], g4_cli_args.block_size, infile1, 0);
  if (retcode < 0)
    goto fb1_cleanup;

  *retcode = ttd_fb_init(&fbs[1], g4_cli_args.block_size, infile2, 0);
  if (retcode < 0)
    goto fb2_cleanup;

  *retcode = ttd_fb_init(&fbs[2], g4_cli_args.block_size, infile3, 0);
  if (retcode < 0)
    goto fb3_cleanup;

  *retcode = ttd_fb_init(&fbs[3], g4_cli_args.block_size, infile4, 0);
  if (retcode < 0)
    goto fb4_cleanup;


  // Read out first records
  for (i = 0; i < 4; i++) {
    times[i] = ttd_fb_pop(&fbs[i]);
  }

  // Loop over files while all four still have records; this is bulk of operation and we can
  // skip a lot of inner-loop conditionals while all four are non-empty
  int num_empty = 0, least_index;
  ttd_t least_time;
  while (num_empty == 0) {
    // Find least time
    least_index = 0;
    least_time = times[0];
    for (i = 1; i < 4; i++) {
      if (times[i] < least_time) {
        least_index = i;
        least_time = times[i];
      }
    }

    // Update correlation with least time
    ttd_ccorr4_update(ccorr, least_index, least_time);
    times[least_index] = ttd_fb_pop(&fbs[least_index]);

    // Check for emptyness
    for (i = 0; i < 4; i++) {
      num_empty += fbs[i].empty;
    }
  }

  // Loop again, this time with extra conditionals for empty buffers.
  int empty[4];
  int start = 1;
  for (i = 0; i < 4; i++) {
    empty[i] = fbs[i].empty;
  }
  while (num_empty < 4) {
    // Find least time
    for (i = 0; i < 4; i++) {
      if (empty[i] == 0) {
        if (start == 1) {
          least_index = i;
          least_time = times[i];
          start = 0;
        }
        else if (times[i] < least_time) {
          least_index = i;
          least_time = times[i];
        }
      }
    }

    // Update correlation with least time
    ttd_ccorr4_update(ccorr, least_index, least_time);
    times[least_index] = ttd_fb_pop(&fbs[least_index]);

    // Check for emptyness
    for (i = 0; i < 4; i++) {
      if (empty[i] == 0) {
        num_empty += fbs[i].empty;
        empty[i] = fbs[i].empty;
      }
    }

    // Reset start variable for next loop
    start = 1;
  }


  fb4_cleanup:
  ttd_fb_cleanup(&fbs[3]);

  fb3_cleanup:
  ttd_fb_cleanup(&fbs[2]);

  fb2_cleanup:
  ttd_fb_cleanup(&fbs[1]);

  fb1_cleanup:
  ttd_fb_cleanup(&fbs[0]);

  return (ccorr);
}

int main(int argc, char *argv[]) {
  int retcode, exitcode = 0;

  retcode = g4_read_cli(argc, argv);

  if (retcode < 0) {
    exitcode = retcode;
    goto cleanup_g4_cli;
  }
  else if (retcode == G4_CLI_EXIT_RETCODE) {
    goto cleanup_g4_cli;
  }

  if (g4_cli_args.verbose) {
    g4_cli_print_options(G4_CLI_PRINTOPTIONS_NOVERBOSE);
  }

  char *infile1 = g4_cli_args.infiles[0];
  char *infile2 = g4_cli_args.infiles[1];
  char *infile3 = g4_cli_args.infiles[2];
  char *infile4 = g4_cli_args.infiles[3];

  char *outfile = g4_cli_args.outfile;

  if (infile1 == NULL) {
    printf("Error: Missing input file 1. Please specify with the '-1' flag.\n");
    exitcode = -1;
    goto cleanup_g4_cli;
  }
  if (infile2 == NULL) {
    printf("Error: Missing input file 2. Please specify with the '-2' flag.\n");
    exitcode = -1;
    goto cleanup_g4_cli;
  }
  if (infile3 == NULL) {
    printf("Error: Missing input file 3. Please specify with the '-3' flag.\n");
    exitcode = -1;
    goto cleanup_g4_cli;
  }

  if (infile4 == NULL) {
    printf("Error: Missing input file 4. Please specify with the '-4' flag.\n");
    exitcode = -1;
    goto cleanup_g4_cli;
  }

  if (outfile == NULL) {
    printf("Error: Missing output file. Please specify with the '-o' flag.\n");
    exitcode = -1;
    goto cleanup_g4_cli;
  }

  ttd_ccorr4_t *g4_ccorr = ttd_g4(infile1, infile2, infile3, infile4, &retcode);

  ttd_ccorr4_write_csv(g4_ccorr, outfile);


  // Allocate the filename for the times csv. '-times.csv'
  char *times_fname = append_before_extension("-times", outfile);

  // Write said times csv and free string from previous line
  ttd_ccorr4_write_times_csv(g4_ccorr, times_fname);
  free(times_fname);

  cleanup_ccorr:
  ttd_ccorr4_cleanup(g4_ccorr);
  free(g4_ccorr);

  cleanup_g4_cli:
  g4_cli_cleanup();

  exit_block:
  exit(exitcode);
}
