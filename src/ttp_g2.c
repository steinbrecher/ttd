#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

#include "ttp.h"
#include "ttp_cli.h"

#include "ttd.h"
#include "ttd_merge.h"
#include "ttd_ringbuffer.h"
#include "ttd_crosscorr.h"

int64_t ttp_g2(doqkd_buffer_t *in1, doqkd_buffer_t *in2) {
  int64_t output_buffer_count = 0;
  uint64_t t1, t2;

  ttd_ccorr_t *ccorr = ttd_ccorr_build(1024, ttp_cli_args.window_time);

  // Initialize buffers
  // TODO: Adapt ttd_doqkd_buffers to be more general (i.e. not dokd-specific)
  // so that its helper functions can be used here and in other functions
  in1->offset = 0;
  in1->buffer_size = PHOTONBLOCK;
  in1->buffer_fill = 0;
  in1->num_read = 0;
  in1->empty = 0;
  in1->buffered_records = (ttd_t *)malloc(in1->buffer_size * sizeof(ttd_t));
  in1->buffer_allocated = 1;

  in2->offset = 0;
  in2->buffer_size=PHOTONBLOCK;
  in2->buffered_records = (ttd_t *)malloc(in2->buffer_size * sizeof(ttd_t));
  in2->buffer_fill = 0;
  in2->num_read = 0;
  in2->empty = 0;
  in2->buffered_records = (ttd_t *)malloc(in1->buffer_size * sizeof(ttd_t));
  in2->buffer_allocated = 1;

  while((in1->empty == 0) && (in2->empty == 0)) {
    if (t1 <= t2) {
      ttd_ccorr_update(ccorr, 0, t1);
      t1 = ttd_buffer_pop(in1);
    }
    else {
      ttd_ccorr_update(ccorr, 1, t2);
      t2 = ttd_buffer_pop(in2);
      output_buffer_count ++;
    }
  }

  while (in1->empty == 0) {
    ttd_ccorr_update(ccorr, 0, t1);
    t1 = ttd_buffer_pop(in1);
  }

  while (in2->empty == 0) {
    ttd_ccorr_update(ccorr, 1, t1);
    t2 = ttd_buffer_pop(in2);
    output_buffer_count ++;
  }

  ttd_ccorr_write_csv(ccorr, ttp_cli_args.outfile1);

  return(0);
}


int main(int argc, char* argv[]) {
  // Write command line input to cli_args struct
  ttp_read_cli(argc, argv);

  doqkd_buffer_t in1;
  doqkd_buffer_t in2;

  // Try to open files
  if ((in1.fp = fopen(ttp_cli_args.infile1, "rb")) == NULL) {
    printf("ERROR: Could not open %s for reading.\n", argv[1]);
    exit(-1);
  }
  in1.file_open = 1;

  if ((in2.fp = fopen(ttp_cli_args.infile2, "rb")) == NULL) {
    printf("ERROR: Could not open %s for reading.\n", argv[2]);
    exit(-1);
  }
  in2.file_open = 1;

  ttp_g2(&in1, &in2);


  exit(0);
}
