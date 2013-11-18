#define __STDC_FORMAT_MACROS
#include <getopt.h>
#include <math.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "cli.h"
#include "timebuffer.h"
#include "hh_header.h"
#include "hhg2.h"

int main(int argc, char* argv[])
{
  FILE *ht_file;
  uint64_t n, m; 

  printf("\n******************************** User Settings *******************************\n");

  // Write command line input to cli_args struct
  read_cli(argc, argv);

  printf("Bin Time: %g ns\n", cli_args.bin_time/1e3);
  printf("Correlation window: %g ns\n", cli_args.correlation_window/1e3);

  printf("\n***************************** Header Information *****************************\n");

  if((ht_file=fopen(argv[argc-1],"rb"))==NULL) 
    { 
      printf("\n ERROR: Input file cannot be opened. Does it exist?\n"); 
      exit(-1);
    }

  if (read_header(ht_file) < 0) // Tries to read the header. Should fail on non-.ht2 or .ht3 files
    {
      fclose(ht_file);
      printf("\n ERROR: Cannot read header. Is %s an HT2 or HT3 file?", argv[1]);
      exit(-1);
    }

  uint64_t channels = MainHardwareHdr.InpChansPresent;

  printf("File Name: %s\n", argv[argc-1]);
  printf("File Mode: T%d\n", BinHdr.MeasMode);
  printf("File Version: %s\n", TxtHdr.FormatVersion);
  printf("Number of channels: %d\n", MainHardwareHdr.InpChansPresent);
  printf("Sync Rate: %g MHz\n", (double)TTTRHdr.SyncRate*1e-6);
  printf("Sync Period: %g ns\n", 1e9/TTTRHdr.SyncRate);
  printf("Resolution: %g ps\n", BinHdr.Resolution);
  printf("Acquisition Time: %g s\n", ((double)BinHdr.Tacq)/1e3);
  printf("Number of Records: %" PRIu64 "\n", TTTRHdr.nRecords);

  clock_t start, diff; 		// Benchmarking timers for read_file function call 
  uint64_t total_read, markers = 0, syncs=0; // Heuristics from read_htX_vY
  
  start = clock();
  total_read = run_g2(ht_file);
  diff = clock() - start;


  double read_time = (double)diff / CLOCKS_PER_SEC;
  printf("Elapsed Time: %g seconds\n", read_time);
  printf("Records Read: %" PRIu64 "\n", total_read);

  printf("\n");


  fclose(ht_file);

  if (total_read != TTTRHdr.nRecords) {
    printf("\nWARNING: Did not reach end of file.\n");
  }

  exit(0);
}
