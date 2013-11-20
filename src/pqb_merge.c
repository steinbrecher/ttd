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
#include "pqb_merge.h"

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

  
  // Run the merge operation
  int ret;
  ret = pqb_merge(&in1, &in2, outfile);
  if (ret  < 0) {
    exit(ret);
  }
  
  exit(0);
}
