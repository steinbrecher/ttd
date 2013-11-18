#ifndef GLOBAL_ARGS_SEEN
#define GLOBAL_ARGS_SEEN

struct global_args_t {
  double bin_time; // '-b' option
  double correlation_window; // '-w' option
  
  int channel_pairs; // Calculated as choose(channels, 2);
} global_args;

static const struct option longopts[] = {
  { "bin-time", required_argument, NULL, 'b'},
  { "correlation-window", required_argument, NULL, 'w'},
};

static const char *optstring = "b:r:w:";

int read_cli(int argc, char* argv[]) 
{
  // Default values
  global_args.bin_time = 10*1000;
  global_args.correlation_window = 100*1000;

  if(((argc % 2) != 0) || (argc == 0)) {
      printf("Usage: read_hh [options] infile\n");
      printf("infile is a HydraHarp .ht2 or .ht3 file (binary)\n"); 
      //getch();
      return(-1);
    }

  int option_index, opt, num_args=0;

  opt = getopt_long( argc, argv, optstring, longopts, &option_index );
  while (opt != -1)
    {
      num_args++;
      switch (opt) 
	{
	case 'b':
	  global_args.bin_time = atof(optarg)*1e3;
	  break;
	case 'w':
	  global_args.correlation_window = atof(optarg)*1e3;
	  break;
	default: 
	  // Shouldn't actually get here
	  break;
	}
      opt = getopt_long( argc, argv, optstring, longopts, &option_index );
    }
  return(num_args);
}
#endif /* GLOBAL_ARGS_SEEN */
