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
#include "correlation.h"
#include "timebuffer.h"
#include "pqb_g2.h"


int main(int argc, char* argv[]) {
  FILE *ht_file;

  // Write command line input to cli_args struct
  read_cli(argc, argv);

  // Try to open the input file
  if((ht_file=fopen(argv[argc-1],"rb"))==NULL) { 
      printf("\n ERROR: Input file cannot be opened. Does it exist?\n"); 
      exit(-1);
    }

  // Try to read the header of the HydraHarp file
  // TODO: Wrap this in a clause for HH mode only
  if (read_header(ht_file) < 0) { 
      fclose(ht_file);
      printf("\n ERROR: Cannot read header. Is %s an HT2 or HT3 file?", argv[1]);
      exit(-1);
    }

  write_g2_properties();

  clock_t start, diff; 		// Benchmarking timers for read_file function call 
  
  start = clock();
  run_g2(ht_file);

  diff = clock() - start;

  double read_time = (double)diff / CLOCKS_PER_SEC;
  printf("Elapsed Time: %g seconds\n", read_time);

  fclose(ht_file);
  exit(0);
}
