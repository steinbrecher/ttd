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

  printf("Bin Time: %g ns\n", global_args.bin_time);
  printf("Correlation window: %g ns\n", global_args.correlation_window);
  //printf("Rate Tracking Window: %g ns\n", global_args.rate_window);

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

  double sync_period = (double)1e9 / (double)TTTRHdr.SyncRate;
  double resolution = ((double)BinHdr.Resolution)*((double)1e-3);
  double Tacq = BinHdr.Tacq * 1e6;
  uint64_t channels = MainHardwareHdr.InpChansPresent;

  printf("File Name: %s\n", argv[argc-1]);
  printf("File Mode: T%d\n", BinHdr.MeasMode);
  printf("File Version: %s\n", TxtHdr.FormatVersion);
  printf("Number of channels: %d\n", MainHardwareHdr.InpChansPresent);
  printf("Sync Rate: %g MHz\n", (double)TTTRHdr.SyncRate*1e-6);
  printf("Sync Period: %g ns\n", sync_period);
  printf("Resolution: %g ps\n", resolution*1e3);
  printf("Acquisition Time: %g ns\n", Tacq);
  printf("Number of Records: %" PRIu64 "\n", TTTRHdr.nRecords);



  int pairs = channels * (channels - 1)/2; // Number of pairs of channels = channels choose 2 
  TimeBufferGroup tbs[channels];       // Container of ring buffers for arrival times 
  CorrelationGroup correlations[pairs]; // Container of correlation tracking objects 

  for (n=0; n < channels; n++) {
    tbInit(&(tbs[n].buffer), n, 1024, global_args.correlation_window, Tacq);
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
  /* if (BinHdr.MeasMode == 2) */
  /*   { */
  /*     if (strcmp(TxtHdr.FormatVersion, "1.0")==0) */
  /* 	{ */
  /* 	  total_read = read_ht2_v1(ht_file, tbs, correlations, &syncs, &markers); */
  /* 	} */
  /*     else if (strcmp(TxtHdr.FormatVersion, "2.0")==0) */
  /* 	{ */
  /* 	  total_read = read_ht2_v2(ht_file, tbs, correlations, &syncs, &markers); */
  /* 	} */
  /*     else */
  /* 	{ */
  /* 	  printf("ERROR: Unrecognized file version '%s'\n", TxtHdr.FormatVersion); */
  /* 	  exit(-1); */
  /* 	} */
  /*   } */
  /* else if (BinHdr.MeasMode == 3) */
  /*   { */
  /*     if (strcmp(TxtHdr.FormatVersion, "1.0")==0) */
  /* 	{ */
  /* 	  printf("Using read_ht3_v1\n"); */
  /* 	  total_read = read_ht3_v1(ht_file, tbs, correlations, &markers); */
  /* 	  printf("Finished reading file.\n"); */
  /* 	} */
  /*     else if (strcmp(TxtHdr.FormatVersion, "2.0")==0) */
  /* 	{ */
  /* 	  printf("Using read_ht3_v2\n"); */
  /* 	  total_read = read_ht3_v2(ht_file, tbs, correlations, &markers); */
  /* 	} */
  /*     else */
  /* 	{ */
  /* 	  printf("ERROR: Unrecognized file version '%s'\n", TxtHdr.FormatVersion); */
  /* 	  exit(-1); */
  /* 	} */
  /*   } */

  diff = clock() - start;
  double read_time = (double)diff / CLOCKS_PER_SEC;

  fclose(ht_file);

  printf("\n*********************************** Results ***********************************\n");
  printf("Elapsed Time: %g seconds\n", read_time);
  printf("Records Read: %" PRIu64 "\n", total_read);
  printf("Syncs Found: %llu\n", syncs);
  printf("Markers Found: %llu\n", markers);

  printf("\n");

  for(n=0; n<channels; n++) {
      printf("Total Photons on Channel %llu: %llu\n", n, tbs[n].buffer.total_counts);
    }
  printf("\n");
  for (n=0; n<pairs; n++) {
    printf("Total Counts in Correlation %d->%d: %llu\n", 
	   correlations[n].corr.chan1, correlations[n].corr.chan2, correlations[n].corr.total);
  }


  double norms[pairs];
  uint64_t total1, total2;

  for (n=0; n<pairs; n++) {
    total1 = tbs[correlations[n].corr.chan1].buffer.total_counts;
    total2 = tbs[correlations[n].corr.chan2].buffer.total_counts;
    norms[n] =  Tacq * global_args.bin_time / (double)(total1 * total2);
    printf("Norm %llu: %g\n", n, norms[n]);
  }

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

  // Outputs the g2 files
  for (n=0; n<pairs; n++) {
    snprintf(fname, sizeof(fname), "g2_%d%d.csv", // Most reliable way to create string from int 
	     correlations[n].corr.chan1, correlations[n].corr.chan2); 
    data_file = fopen(fname,"wb");

    for (m=0; m < correlations[n].corr.num_bins; m++) {
      fprintf(data_file, "%g, %g\n", ((m*global_args.bin_time)-global_args.correlation_window), 
	      (double)(correlations[n].corr.hist[m].counts)*norms[n]);
    }

    fclose(data_file);
  }

  if (total_read != TTTRHdr.nRecords) {
    printf("\nWARNING: Did not reach end of file.\n");
  }

  exit(0);
}
