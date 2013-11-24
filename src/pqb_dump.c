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

#ifndef PHOTONBLOCK
#define PHOTONBLOCK 32768
#endif

int main (int argc, char* argv[]) {
  FILE *pqb_file;

  // Try to open the input file
  if((pqb_file = fopen(argv[argc-1],"rb")) == NULL) { 
      printf("\n ERROR: Input file cannot be opened.\n"); 
      exit(-1);
    }

  pqb_t *file_block = (pqb_t *) malloc(PHOTONBLOCK*sizeof(pqb_t));
  uint64_t k, num_photons=PHOTONBLOCK;

  while (num_photons == PHOTONBLOCK) {
    num_photons = fread(file_block, sizeof(pqb_t), PHOTONBLOCK, pqb_file);
    for (k=0; k<num_photons; k++) {
      printf("%" PRIu64 "\n", file_block[k]);
    }
  }
  exit(0);
  
}
