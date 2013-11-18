#define __STDC_FORMAT_MACROS
#include <getopt.h>
#include <math.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "global_args.h"
#include "timebuffer.h"
#include "hh_header.h"
#include "hhg2.h"

int main(int argc, char* argv[])
{
  FILE *ht_file;
  uint64_t n, m; 

  printf("\n******************************** User Settings *******************************\n");

  // Write command line input to global_args
  read_cli(argc, argv);

  printf("Bin Time: %g ns\n", global_args.bin_time/1e3);
  printf("Correlation window: %g ns\n", global_args.correlation_window/1e3);
  //printf("Rate Tracking Window: %g ns\n", global_args.rate_window*1e3);

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
  printf("Acquisition Time: %g ms\n", (double)BinHdr.Tacq);
  printf("Number of Records: %" PRIu64 "\n", TTTRHdr.nRecords);

  int pairs = channels * (channels - 1)/2; // Number of pairs of channels = channels choose 2 
  TimeBufferGroup tbs[channels];       // Container of ring buffers for arrival times 
  CorrelationGroup correlations[pairs]; // Container of correlation tracking objects 

  

  for (n=0; n < channels; n++) {
    tbInit(&(tbs[n].buffer), n, 4096, global_args.correlation_window);
  }

  for (n=0; n < channels-1; n++) 
    {
      for (m=n+1; m < channels; m++) 
	{
	  corrInit(&(correlations[pair_lookup(n,m)].corr), n, m);
	}
    }
  clock_t start, diff; 		// Benchmarking timers for read_file function call 
  uint64_t total_read, markers = 0, syncs=0; // Heuristics from read_htX_vY
  
  start = clock();
  total_read = run_g2(ht_file, tbs, correlations);
  diff = clock() - start;


  double read_time = (double)diff / CLOCKS_PER_SEC;

  fclose(ht_file);

  printf("\n*********************************** Results ***********************************\n");
  printf("Elapsed Time: %g seconds\n", read_time);
  printf("Records Read: %" PRIu64 "\n", total_read);

  printf("\n");

  for(n=0; n<channels; n++) {
      printf("Total Photons on Channel %" PRIu64 ": %" PRIu64 "\n", n, tbs[n].buffer.total_counts);
    }
  printf("\n");
  for (n=0; n<pairs; n++) {
    printf("Total Counts in Correlation %d->%d: %" PRIu64 "\n", 
	   correlations[n].corr.chan1, correlations[n].corr.chan2, correlations[n].corr.total);
  }


  double norms[pairs];
  uint64_t total1, total2;

  FILE *data_file;
  char fname[80];

  // Outputs the hist files
  for (n=0; n<pairs; n++) {
    snprintf(fname, sizeof(fname), "hist_%d%d.csv", // Most reliable way to create string from int 
	     correlations[n].corr.chan1, correlations[n].corr.chan2); 
    data_file = fopen(fname,"wb");

    for (m=0; m < correlations[n].corr.num_bins; m++) {
      fprintf(data_file, "%g, %g\n", 
	      ((m*global_args.bin_time)-global_args.correlation_window), (double)(correlations[n].corr.hist[m].counts));
    }

    fclose(data_file);
  }

  if (total_read != TTTRHdr.nRecords) {
    printf("\nWARNING: Did not reach end of file.\n");
  }

  exit(0);
}
