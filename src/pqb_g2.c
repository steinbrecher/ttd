#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

#include "pqb.h"
#include "timebuffer.h"
#include "pqb_merge.h"
#include "pqb_g2.h"

void pqb_g2_push(pqb_g2_correlation_t *correlation, int channel, uint64_t time) {
  int other_channel;
  if (channel == 1) {
    pqb_g2_tb_write(correlation->tb1, time);
    other_channel = 2;
  }
  else if (channel == 2) {
    pqb_g2_tb_write(correlation->tb2, time);
    other_channel = 1;
  }
  pqb_g2_tb_prune(correlation->tb1, time);
  pqb_g2_tb_prune(correlation->tb2, time);
  pqb_g2_correlation_update(correlation, other_channel, time);
}

int64_t pqb_g2(doqkd_buffer_t *in1, doqkd_buffer_t *in2) {
  int64_t output_buffer_count = 0;
  uint64_t t1, t2;

  pqb_g2_timebuffer_t tb1, tb2;
  pqb_g2_tb_init(&tb1, 1, 1024, pqb_g2_cli_args.correlation_window);
  pqb_g2_tb_init(&tb2, 2, 1024, pqb_g2_cli_args.correlation_window);

  pqb_g2_correlation_t correlation;
  pqb_g2_corr_init(&correlation, &tb1, &tb2);

  // Initialize buffers
  // TODO: Adapt pqb_doqkd_buffers to be more general (i.e. not dokd-specific)
  // so that its helper functions can be used here and in other functions
  in1->offset = 0;
  in1->buffer_size = PHOTONBLOCK;
  in1->buffer_fill = 0;
  in1->num_read = 0;
  in1->empty = 0;
  in1->buffered_records = (pqb_t *)malloc(in1->buffer_size * sizeof(pqb_t));
  in1->buffer_allocated = 1;

  in2->offset = 0;
  in2->buffer_size=PHOTONBLOCK;
  in2->buffered_records = (pqb_t *)malloc(in2->buffer_size * sizeof(pqb_t));
  in2->buffer_fill = 0;
  in2->num_read = 0;
  in2->empty = 0;
  in2->buffered_records = (pqb_t *)malloc(in1->buffer_size * sizeof(pqb_t));
  in2->buffer_allocated = 1;

  while((in1->empty == 0) && (in2->empty == 0)) {
    if (t1 <= t2) {
      pqb_g2_push(&correlation, 1, t1);
      t1 = pqb_buffer_pop(in1);
    }
    else {
      pqb_g2_push(&correlation, 2, t2);
      t2 = pqb_buffer_pop(in2);
      output_buffer_count ++;
    }
  }

  while (in1->empty == 0) {
    pqb_g2_push(&correlation, 1, t1);
    t1 = pqb_buffer_pop(in1);
  }

  while (in2->empty == 0) {
    pqb_g2_push(&correlation, 2, t1);
    t2 = pqb_buffer_pop(in2);
    output_buffer_count ++;
  }

  pqb_g2_correlation_output(&correlation);

  return(0);
}


int main(int argc, char* argv[]) {
  // Write command line input to cli_args struct
  pqb_g2_read_cli(argc, argv);

  doqkd_buffer_t in1;
  doqkd_buffer_t in2;
  FILE *outfile;

  // Try to open files
  if ((in1.fp = fopen(pqb_g2_cli_args.infile1, "rb")) == NULL) {
    printf("ERROR: Could not open %s for reading.\n", argv[1]);
    exit(-1);
  }
  in1.file_open = 1;

  if ((in2.fp = fopen(pqb_g2_cli_args.infile2, "rb")) == NULL) {
    printf("ERROR: Could not open %s for reading.\n", argv[2]);
    exit(-1);
  }
  in2.file_open = 1;

  pqb_g2(&in1, &in2);


  exit(0);
}
