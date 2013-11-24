#ifndef PQB_G2_HEADER_SEEN
#define PQB_G2_HEADER_SEEN

#include "pqb.h"

/*
struct cli_args_t {
  uint64_t bin_time; // -b
  uint64_t window_time; // -w
  char infile1[80]; // -i
  char infile2[80]; // -I
  char outfile1[80]; // -o
} ttp_cli_args;

static const struct option pqb_g2_longopts[] = {
  { "bin-time", required_argument, NULL, 'b'},
  { "correlation-window", required_argument, NULL, 'w'},
  { "input-1", required_argument, NULL, 'i' },
  { "input-2", required_argument, NULL, 'I' },
  { "output", required_argument, NULL, 'o' },
};

static const char *pqb_g2_optstring = "b:w:i:I:o:";

int pqb_g2_read_cli(int argc, char* argv[])  {
  // Default values
  ttp_cli_args.bin_time = 100;
  ttp_cli_args.window_time = 10000;
  strcpy(ttp_cli_args.infile1, "");
  strcpy(ttp_cli_args.infile2, "");
  strcpy(ttp_cli_args.outfile1, "cross_corr.csv");

  int option_index, opt, num_args=0;

  opt = getopt_long( argc, argv, pqb_g2_optstring, pqb_g2_longopts, &option_index );
  while (opt != -1) {
    num_args++;
    switch (opt) {
    case 'b':
      ttp_cli_args.bin_time = atof(optarg);
      break;
    case 'w':
      ttp_cli_args.window_time = atof(optarg);
      break;
    case 'i':
      strcpy(ttp_cli_args.infile1, optarg);
      break;
    case 'I':
      strcpy(ttp_cli_args.infile2, optarg);
      break;
    case 'o':
      strcpy(ttp_cli_args.outfile1, optarg);
      break;
    default: 
      // Shouldn't actually get here
      break;
    }
    opt = getopt_long( argc, argv, pqb_g2_optstring, pqb_g2_longopts, &option_index );
  }

  printf("\n******************************** User Settings *******************************\n");
  printf("Bin Time: %" PRIu64 " ps\n", (uint64_t)ttp_cli_args.bin_time);
  printf("Correlation Window: %" PRIu64 " ps\n", (uint64_t)ttp_cli_args.window_time);
  printf("Input file 1: %s\n", ttp_cli_args.infile1);
  printf("Input file 2: %s\n", ttp_cli_args.infile2);
  printf("Output file: %s\n", ttp_cli_args.outfile1);
  return(0);
}
*/

typedef struct {
  int size;		      // Max number of elems 
  int start;		      // Index of oldest element 
  unsigned int count;	      // Number of active elements 
  unsigned int channel;	      // Channel this buffer corresponds to 

  uint64_t total_counts; // Tally up all counts on this channel 

  uint64_t duration;
  uint64_t *times;
} pqb_g2_timebuffer_t;

void pqb_g2_tb_init(pqb_g2_timebuffer_t *tb, int channel, int size, uint64_t duration) {
  tb->size  = size;
  tb->start = 0;
  tb->count = 0;
  tb->channel = channel;

  tb->total_counts = 0;

  tb->duration = duration;

  tb->times = (uint64_t *)malloc(tb->size * sizeof(TimeType));
}

uint64_t pqb_g2_tb_get(pqb_g2_timebuffer_t *tb, int offset) {
  return(tb->times[(tb->start + offset) % tb->size]);
}

void pqb_g2_tb_write(pqb_g2_timebuffer_t *tb, uint64_t time) {
  ++ tb->total_counts;
  int end = (tb->start + tb->count) % tb->size;

  tb->times[end] = time;

  ++ tb->count;
}

void pqb_g2_tb_prune(pqb_g2_timebuffer_t *tb, uint64_t time) {
  while (tb->count > 0) {
    if ((time - tb->times[tb->start]) > tb->duration) {
      tb->start = (tb->start + 1) % tb->size;
      -- tb->count;
    }
    else break;
  }
}

typedef struct {
  int chan1;
  int chan2;

  pqb_g2_timebuffer_t *tb1;
  pqb_g2_timebuffer_t *tb2;
  
  int num_bins;
  int center_bin;

  uint64_t window_time;
  uint64_t bin_time;

  uint64_t total;
  
  uint64_t *hist;
} pqb_g2_correlation_t;

void pqb_g2_corr_init(pqb_g2_correlation_t *corr, pqb_g2_timebuffer_t *tb1, pqb_g2_timebuffer_t *tb2) {
  int num_bins;
  uint64_t window_time = ttp_cli_args.window_time;
  uint64_t bin_time = ttp_cli_args.bin_time;

  corr->tb1 = tb1;
  corr->tb2 = tb2;

  corr->chan1 = tb1->channel;
  corr->chan2 = tb2->channel;

  corr->num_bins = (int)(2*round(window_time / bin_time) + 1);
  corr->center_bin = (corr->num_bins - 1)/2;
  corr->window_time = window_time;
  corr->bin_time = bin_time;
  
  corr->total = 0;
  corr->hist = (uint64_t *)calloc(corr->num_bins, sizeof(pqb_t));
}

void pqb_g2_correlation_update(pqb_g2_correlation_t *corr, int new_chan, uint64_t new_time) {
  int n, sign=1;
  int64_t delta;
  int delta_b;
  pqb_g2_timebuffer_t *tb;
  if (new_chan == 1) {
    tb = corr->tb1;
  }
  else if (new_chan == 2) {
    tb = corr->tb2;
  }

  if (tb->count > 0) {
    // This ensures that deltaT is (T2 - T1) even when (new_chan == chan2)
    // Strangely, !=chan1 seems to result in ~5% faster runtime than ==chan2
    if (new_chan != corr->chan1) { 
	sign = -1;
      }
    
    for (n=0; n < tb->count; n++) {
      delta = new_time - pqb_g2_tb_get(tb, n);
      delta_b = (int)(corr->center_bin + sign*(delta / corr->bin_time));
      ++ corr->hist[delta_b];
      ++ corr->total;
    }
    
  }
}

void pqb_g2_correlation_output(pqb_g2_correlation_t *correlation) {
  FILE *output_file;
  output_file = fopen(ttp_cli_args.outfile1, "wb");
  int m;
  int64_t window_time = ttp_cli_args.window_time;
  int64_t bin_time = ttp_cli_args.bin_time;

  printf("Bin Time: %" PRId64 "\n", bin_time);
  printf("Correlation Window: %" PRId64 "\n", window_time);
  
  for (m=0; m < correlation->num_bins; m++) {
    fprintf(output_file, "%" PRId64", %" PRIu64 "\n", 
	    ((m*bin_time) - window_time), (correlation->hist[m]));
  }	
  fclose(output_file);
}



#endif //PQ_G2_HEADER_SEEN
