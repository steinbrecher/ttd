#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

#ifndef PHOTONBLOCK
#define PHOTONBLOCK 16384
#endif

#include "pqb.h"
#include "pqb_doqkd_buffers.h"

void pqb_merge_output(uint64_t *records, int64_t num_records, FILE *outfile) {
  fwrite(records, sizeof(uint64_t), num_records, outfile); 
}

int64_t pqb_merge(doqkd_buffer_t *in1, doqkd_buffer_t *in2, FILE *outfile) {
  uint64_t output_buffer[PHOTONBLOCK];
  int64_t output_buffer_count = 0;
  uint64_t t1, t2;
  t1 = pqb_buffer_pop(in1);
  t2 = pqb_buffer_pop(in2);
  while((in1->empty == 0) && (in2->empty == 0)) {
    if (t1 <= t2) {
      output_buffer[output_buffer_count] = t1;
      t1 = pqb_buffer_pop(in1);
      output_buffer_count ++;
    }
    else {
      output_buffer[output_buffer_count] = t2;
      t2 = pqb_buffer_pop(in2);
      output_buffer_count ++;
    }
    if (output_buffer_count == PHOTONBLOCK) {
      pqb_merge_output(output_buffer, PHOTONBLOCK, outfile);
      output_buffer_count = 0;
    }
  }
  while (in1->empty == 0) {
    output_buffer[output_buffer_count] = t1;
    t1 = pqb_buffer_pop(in1);
    output_buffer_count ++;
    if (output_buffer_count == PHOTONBLOCK) {
      pqb_merge_output(output_buffer, PHOTONBLOCK, outfile);
      output_buffer_count = 0;
    }
  }
  while (in2->empty == 0) {
    output_buffer[output_buffer_count] = t2;
    t2 = pqb_buffer_pop(in2);
    output_buffer_count ++;
    if (output_buffer_count == PHOTONBLOCK) {
      pqb_merge_output(output_buffer, PHOTONBLOCK, outfile);
      output_buffer_count = 0;
    }
  }
  if (output_buffer_count > 0) {
    pqb_merge_output(output_buffer, output_buffer_count, outfile);
  }
  

  return(0);
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    printf("ERROR: Incorrect number of inputs.\n");
    printf("Usage: %s infile1 infile2 outfile", argv[0]);
  }
  doqkd_buffer_t in1;
  doqkd_buffer_t in2;
  FILE *outfile;

  printf(argv[1], "rb");
  
  // Try to open files
  if ((in1.fp = fopen(argv[1], "rb")) == NULL) {
    printf("ERROR: Could not open %s for reading.\n", argv[1]);
    exit(-1);
  }
  in1.file_open = 1;

  if ((in2.fp = fopen(argv[2], "rb")) == NULL) {
    printf("ERROR: Could not open %s for reading.\n", argv[2]);
    exit(-1);
  }
  in2.file_open = 1;

  if ((outfile = fopen(argv[3], "wb")) == NULL) {
      printf("ERROR: Could not open %s for writing.\n", argv[3]);
    }

  // Initialize buffers
  // TODO: Adapt pqb_doqkd_buffers to be more general (i.e. not dokd-specific)
  // so that its helper functions can be used here and in other functions
  in1.offset = 0;
  in1.buffer_size = PHOTONBLOCK;
  in1.buffer_fill = 0;
  in1.num_read = 0;
  in1.empty = 0;
  in1.buffered_records = (pqb_t *)malloc(in1.buffer_size * sizeof(pqb_t));
  in1.buffer_allocated = 1;

  in2.offset = 0;
  in2.buffer_size=PHOTONBLOCK;
  in2.buffered_records = (pqb_t *)malloc(in2.buffer_size * sizeof(pqb_t));
  in2.buffer_fill = 0;
  in2.num_read = 0;
  in2.empty = 0;
  in2.buffered_records = (pqb_t *)malloc(in1.buffer_size * sizeof(pqb_t));
  in2.buffer_allocated = 1;
  
  // Run the merge operation
  int ret;
  ret = pqb_merge(&in1, &in2, outfile);
  if (ret  < 0) {
    exit(ret);
  }
  
  exit(0);
}
