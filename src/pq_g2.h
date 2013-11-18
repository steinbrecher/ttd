#ifndef PQ_G2_HEADER_SEEN
#define PQ_G2_HEADER_SEEN

struct cli_args_t {
  double bin_time; // '-b' option
  double correlation_window; // '-w' option
  int channel_pairs; // Calculated as choose(channels, 2);
} cli_args;

static const struct option longopts[] = {
  { "bin-time", required_argument, NULL, 'b'},
  { "correlation-window", required_argument, NULL, 'w'},
};

static const char *optstring = "b:w:";

int read_cli(int argc, char* argv[])  {
  // Default values
  cli_args.bin_time = 10*1000;
  cli_args.correlation_window = 100*1000;

  if(((argc % 2) != 0) || (argc == 0)) {
      printf("Usage: read_hh [options] infile\n");
      printf("infile is a HydraHarp .ht2 or .ht3 file (binary)\n"); 
      return(-1);
    }

  int option_index, opt, num_args=0;

  opt = getopt_long( argc, argv, optstring, longopts, &option_index );
  while (opt != -1) {
      num_args++;
      switch (opt) {
	case 'b':
	  cli_args.bin_time = atof(optarg)*1e3;
	  break;
	case 'w':
	  cli_args.correlation_window = atof(optarg)*1e3;
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
  //  printf("Running in %s mode\n", cli_args.mode);
  return(num_args);
}

#endif //PQ_G2_HEADER_SEEN
