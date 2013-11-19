#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

#include "pq_convert.h"
#include "hh_header.h"

int main(int argc, char* argv[]) {
  FILE *ht_file;

  // Try to open the input file
  if((ht_file = fopen(argv[argc-1],"rb")) == NULL) { 
      printf("\n ERROR: Input file cannot be opened.\n"); 
      exit(-1);
    }

  if (read_header(ht_file) < 0) { 
      fclose(ht_file);
      printf("\n ERROR: Cannot read header. Is %s an HT2 or HT3 file?", argv[1]);
      exit(-1);
    }
  write_convert_properties();

  // Benchmarking timers
  clock_t start, diff; 		

  printf("\n");
  start = clock();
  run_hh_convert(ht_file);
  diff = clock() - start;

  double read_time = (double)diff / CLOCKS_PER_SEC;
  printf("Elapsed Time: %g seconds\n", read_time);

  fclose(ht_file);
  exit(0);
}
