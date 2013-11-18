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

  if (tb->count == tb->size) {
    tb->start = (tb->start + 1) % tb->size; 
    ++ tb->overwrites;
  }
  else
    ++ tb->count;
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



#endif /* TIMEBUFFER_SEEN */
