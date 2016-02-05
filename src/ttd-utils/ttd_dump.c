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

#ifndef PHOTONBLOCK
#define PHOTONBLOCK 32768
#endif

int main (int argc, char* argv[]) {
  FILE *ttd_file;

  // Try to open the input file
  if((ttd_file = fopen(argv[argc-1],"rb")) == NULL) { 
      printf("\n ERROR: Input file cannot be opened.\n"); 
      exit(-1);
    }

  ttd_t *file_block = (ttd_t *) malloc(PHOTONBLOCK*sizeof(ttd_t));
  uint64_t k, num_photons=PHOTONBLOCK;

  while (num_photons == PHOTONBLOCK) {
    num_photons = fread(file_block, sizeof(ttd_t), PHOTONBLOCK, ttd_file);
    for (k=0; k<num_photons; k++) {
      printf("%" PRIu64 "\n", file_block[k]);
    }
  }
  exit(0);
  
}
