#ifndef CORRELATION_SEEN
#define CORRELATION_SEEN

typedef struct { uint64_t counts; } Histogram;

typedef struct {
  int chan1;
  int chan2;
  
  int num_bins;
  int center_bin;

  double correlation_window;
  double bin_time;

  uint64_t total;
  
  Histogram *hist;
} Correlation;

typedef struct { Correlation corr; } CorrelationGroup;

void corrInit(Correlation *corr, int chan1, int chan2) {
  int num_bins;
  double correlation_window = g2_properties.correlation_window;
  double bin_time = g2_properties.bin_time;

  corr->chan1 = chan1;
  corr->chan2 = chan2;

  corr->num_bins = (int)(2*round(correlation_window / bin_time) + 1);
  corr->center_bin = (corr->num_bins - 1)/2;
  corr->correlation_window = g2_properties.correlation_window;
  corr->bin_time = g2_properties.bin_time;
  
  corr->total = 0;
  corr->hist = (Histogram *)malloc(corr->num_bins * sizeof(Histogram));
}

void correlationUpdate(Correlation *corr, TimeBuffer *tb, int new_chan, double new_time) {
  int n, sign=1;
  double delta;
  int delta_b;
  
  
  if (tb->count > 0) {
    // This ensures that deltaT is (T2 - T1) even when (new_chan == chan2)
    // Strangely, !=chan1 seems to result in ~5% faster runtime than ==chan2
    if (new_chan != corr->chan1) { 
	sign = -1;
      }
    
    for (n=0; n < tb->count; n++) {
      delta = new_time - tbGet(tb, n);
      delta_b = (int)(corr->center_bin + sign*round(delta / corr->bin_time));
      ++ corr->hist[delta_b].counts;
      ++ corr->total;
    }
  }
}


#endif // CORRELATION_SEEN
