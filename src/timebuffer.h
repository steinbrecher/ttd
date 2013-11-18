#ifndef TIMEBUFFER_SEEN
#define TIMEBUFFER_SEEN

#include "global_args.h"


typedef struct { 
  double time;
} TimeType;

typedef struct {
  uint64_t counts;
} RateType;

// Circular buffer to hold photon counts 
typedef struct {
  int size;		      // Max number of elems 
  int start;		      // Index of oldest element 
  unsigned int count;	      // Number of active elements 
  unsigned int channel;	      // Channel this buffer corresponds to 
  int overwrites;      // Track overwrites. This should always be 0 

  uint64_t total_counts; // Tally up all counts on this channel 

  double duration; 		// Time photons are active for 
  double rate_window;		// How long to integrate counts for long term tracking 

  TimeType   *times;		// Ring buffer vector 
  RateType *rates;		// Long term rate vector 
} TimeBuffer;

typedef struct { TimeBuffer buffer; } TimeBufferGroup;

void tbInit(TimeBuffer *tb, int channel, int size, double duration, double Tacq) {
  tb->channel = channel;
  tb->size  = size;
  tb->start = 0;
  tb->count = 0;
  
  tb->total_counts = 0;

  tb->duration = duration;
  tb->rate_window = global_args.rate_window;
  

  tb->times = (TimeType *)malloc(tb->size * sizeof(TimeType));
  tb->rates = (RateType *)malloc((int64_t)(Tacq / global_args.rate_window + 0.5) * sizeof(RateType));
}

double tbGet(TimeBuffer *tb, int offset) {
  return(tb->times[(tb->start + offset) % tb->size].time);
}

double tbLastTime(TimeBuffer *tb) {
  return(tb->times[(tb->start + tb->count - 1) % tb->size].time);
}
 
void tbWrite(TimeBuffer *tb, double time) {
  ++ tb->rates[(int64_t)floor(time / tb->rate_window + 0.5)].counts;
  ++ tb->total_counts;

  int end = (tb->start + tb->count) % tb->size;

  tb->times[end] = (TimeType){ time };

  // Move the tail at each insertion to avoid having to check 
  if ((time - tb->times[tb->start].time) > tb->duration) {
    tb->start = (tb->start + 1) % tb->size;
    -- tb->count;
  }

  // NOTE: NEED TO UPDATE THIS TO RESIZE BUFFER IN CASE OF OVERFLOW
  if (tb->count == tb->size) {
    tb->start = (tb->start + 1) % tb->size; 
    ++ tb->overwrites;
  }
  else
    ++ tb->count;
  // Should probably return pointer to timebuffer in case of resize here.
}

void tbPrune(TimeBuffer *tb, double time) {
  if (tb->count == 0) {
  }
  while (tb->count > 0) {
    if ((time - tb->times[tb->start].time) > tb->duration) {
      tb->start = (tb->start + 1) % tb->size;
      -- tb->count;
    }
    else break;
  }
}

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

void corrInit(Correlation *corr, int chan1, int chan2)
{
  int num_bins;
  corr->chan1 = chan1;
  corr->chan2 = chan2;
  
  num_bins = 2*(int) floor(global_args.correlation_window / global_args.bin_time) + 1;
  corr->num_bins = num_bins;
  corr->center_bin = (corr->num_bins - 1)/2;
  corr->correlation_window = global_args.correlation_window;
  corr->bin_time = global_args.bin_time;
  
  corr->total = 0;
  corr->hist = (Histogram *)malloc(num_bins * sizeof(Histogram));
}

void correlationUpdate(Correlation *corr, TimeBuffer *tb, int new_chan, 
		       double new_time)
{
  int n, sign=1;
  double delta, delta_b;
  
  if (tb->count > 0) {
    // This ensures that deltaT is (T2 - T1) even when (new_chan == chan2)
    // Strangely, !=chan1 seems to result in ~5% faster runtime than ==chan2
    if (new_chan != corr->chan1) { 
	sign = -1;
      }

    for (n=0; n < tb->count; n++) {
      delta = sign * (new_time - tbGet(tb, n));
      delta_b = floor(corr->center_bin + (delta / corr->bin_time) + 0.5);
	++ corr->hist[(int)delta_b].counts;
	++ corr->total;
    }
  }
}


#endif /* TIMEBUFFER_SEEN */
