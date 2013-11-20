#ifndef PQB_G2_HEADER_SEEN
#define PQB_G2_HEADER_SEEN

struct cli_args_t {
  int64_t bin_time; // -b
  int64_t correlation_window; // -w
  char infile1[80]; // -i
  char infile2[80]; // -I
  char outfile[80]; // -o
} cli_args;

static const struct option longopts[] = {
  { "bin-time", required_argument, NULL, 'b'},
  { "correlation-window", required_argument, NULL, 'w'},
  { "chan1-in", required_argument, NULL, 'i' },
  { "chan2-in", required_argument, NULL, 'I' },
  { "output", required_argument, NULL, 'o' },
};

static const char *optstring = "b:w:";

int read_cli(int argc, char* argv[])  {
  // Default values
  cli_args.bin_time = 100;
  cli_args.correlation_window = 10000;
  strcpy(cli_args.infile1, "");
  strcpy(cli_args.infile2, "");
  strcpy(cli_args.outfile, "cross_corr.csv");

  int option_index, opt, num_args=0;

  opt = getopt_long( argc, argv, optstring, longopts, &option_index );
  while (opt != -1) {
    num_args++;
    switch (opt) {
    case 'b':
      cli_args.bin_time = atof(optarg);
      break;
    case 'w':
      cli_args.correlation_window = atof(optarg);
      break;
    case 'i':
      strcpy(cli_args.infile1, optarg);
      break;
    case 'I':
      strcpy(cli_args.infile2, optarg);
      break;
    case 'o':
      strcpy(cli_args.outfile, optarg);
      break;
    default: 
      // Shouldn't actually get here
      break;
    }
    opt = getopt_long( argc, argv, optstring, longopts, &option_index );
  }

  printf("\n******************************** User Settings *******************************\n");
  printf("Bin Time: %g ps\n", cli_args.bin_time);
  printf("Correlation Window: %g ps\n", cli_args.correlation_window);
  return(num_args);
}

void g2_insert(double realtime, int channel, TimeBufferGroup *tbs, Correlation *correlation) {
  tbWrite(&(tbs[channel].buffer), realtime);

  int m;
  for (m=0; m<4; m++) {
    tbPrune(&(tbs[m].buffer), realtime);
  }

  int other_chan = channel-1;
  correlationUpdate(correlation, 
		    &(tbs[other_chan].buffer), 
		    channel, 
		    realtime);
}

void output_g2_csv(Correlation *correlation) {
  FILE *data_file;
  char fname[80];
  int n;
  uint64_t m;
  int64_t bin_time = cli_args.bin_time;
  int64_t correlation_window = cli_args.correlation_window;

  data_file = fopen(cli_args.outfile,"wb");
  
  fprintf(data_file, "%g, %g\n", ((m*bin_time) - correlation_window), (double)(correlation->corr.hist[m].counts));

  fclose(data_file);
}

uint64_t run_g2(FILE *fpin) { 
  tTRec TRec;
  uint64_t n, m, num_photons, total_read=0;

  double overflow_correction=0;
  double correlation_window = g2_properties.correlation_window;


  // Read out necessary g2_properties
  int channels = g2_properties.channels;
  int pairs = g2_properties.channel_pairs;
  int meas_mode = g2_properties.meas_mode;
  int file_format_version = g2_properties.file_format_version;

  TimeBufferGroup tbs[channels]; // Container of ring buffers for arrival times 
  CorrelationGroup correlations[pairs]; // Container of correlation tracking objects 

  // Note: Need to free tb->times for each of these when done
  for (n=0; n < channels; n++) {
    tbInit(&(tbs[n].buffer), n, 4096, correlation_window);
  }

  // Note: Need to free corr->hist for each of these when done
  for (n=0; n < channels-1; n++) {
      for (m=n+1; m < channels; m++) {
	  corrInit(&(correlations[pair_lookup(n,m)].corr), n, m);
	}
    }

  // Allocate memory for mapping blocks of the input file
  tTRec *file_block = (tTRec *) malloc(PHOTONBLOCK*sizeof(TRec.allbits));

  // Function pointer to the generic g2 function call (mode and version dependent)
  void (*g2)(tTRec, double *, TimeBufferGroup *, CorrelationGroup *);

  // Select which function to use to g2 photon records
  if (meas_mode == 2) {
    if (file_format_version == 1) {
      g2 = &ht2_v1_g2;
    }
    else if (file_format_version == 2) {
      g2 = &ht2_v2_g2;
    }
    else {
      return(-1);
    }
  }
  else if (meas_mode == 3) {
    if (file_format_version == 1) {
      g2 = &ht3_v1_g2;
    }
    else if (file_format_version == 2) {
      g2 = &ht3_v2_g2;
    }
    else {
      return(-1);
    }
  }

  num_photons = PHOTONBLOCK;
  while (num_photons == PHOTONBLOCK) {
    num_photons = fread(file_block, sizeof(TRec.allbits), PHOTONBLOCK, fpin);

    for (n=0; n < num_photons; n++) {
      total_read++;
      g2(file_block[n], &overflow_correction, tbs, correlations);
    }
  }

  // Output Statistics
  printf("\n*********************************** g2 Results ***********************************\n");
  for(n=0; n<channels; n++) {
      printf("Total Photons on Channel %" PRIu64 ": %" PRIu64 "\n", n, tbs[n].buffer.total_counts);
    }
  printf("\n");
  for (n=0; n<pairs; n++) {
    printf("Total Counts in Correlation %d->%d: %" PRIu64 "\n", 
	   correlations[n].corr.chan1, correlations[n].corr.chan2, correlations[n].corr.total);
  }

  // Write to files
  output_g2_csv(correlations);

  // Free allocated memory
  free(file_block);

  for (n=0; n < channels; n++) {
    free(tbs[n].buffer.times);
  }

  for (n=0; n < channels-1; n++) {
      for (m=n+1; m < channels; m++) {
	free(correlations[pair_lookup(n,m)].corr.hist);
	}
    }

  printf("Records Read: %" PRIu64 "\n", total_read);
  if (total_read != TTTRHdr.nRecords) {
    printf("\nWARNING: Did not reach end of file.\n");
  }


  
  return(total_read);
}


#endif //PQ_G2_HEADER_SEEN
