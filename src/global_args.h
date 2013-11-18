#ifndef GLOBAL_ARGS_SEEN
#define GLOBAL_ARGS_SEEN

struct global_args_t {
  double bin_time; // '-b' option
  double correlation_window; // '-c' option
  int pages; // '-p' option
  double rate_window; // '-r' option
} global_args;

static const struct option longopts[] = {
  { "bin-time", required_argument, NULL, 'b'},
  { "corr-window", required_argument, NULL, 'c'},
  { "rate-window", required_argument, NULL, 'r'},
};


static const char *optstring = "b:c:r:";

int read_cli(int argc, char* argv[]) 
{
  // Default values
  global_args.bin_time = 10;
  global_args.correlation_window = 100;
  global_args.rate_window = 1e8;

  if((argc % 2) != 0)
    {
      printf("Usage: read_hh -b [bin_time] -c [corr_window] infile\n");
      printf("infile is a HydraHarp ht2 or ht3 file (binary)\n"); 
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
	  global_args.bin_time = atof(optarg);
	  break;
	case 'c':
	  global_args.correlation_window = atof(optarg);
	  break;
	case 'r':
	  global_args.rate_window = atof(optarg);
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
