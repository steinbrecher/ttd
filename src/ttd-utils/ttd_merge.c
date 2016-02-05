#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

#include "ttd.h"
#include "ttd_filebuffer.h"
#include "ttd_merge.h"

int main(int argc, char* argv[]) {
  if (argc != 4) {
    printf("ERROR: Incorrect number of inputs.\n");
    printf("Usage: %s infile1 infile2 outfile", argv[0]);
  }
  FILE *outfile;

  if ((outfile = fopen(argv[3], "wb")) == NULL) {
      printf("ERROR: Could not open %s for writing.\n", argv[3]);
    }

  
  // Run the merge operation
  int ret;
  ret = ttd_merge(argv[1], argv[2], outfile);
  if (ret  < 0) {
    exit(ret);
  }
  
  exit(0);
}
