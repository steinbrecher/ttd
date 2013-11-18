#ifndef TIMEBUFFER_SEEN
#define TIMEBUFFER_SEEN

#include "cli.h"


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

  uint64_t total_counts; // Tally up all counts on this channel 

  double duration; 		// Time photons are active for 

  TimeType   *times;		// Ring buffer vector 
} TimeBuffer;

typedef struct { TimeBuffer buffer; } TimeBufferGroup;

void tbInit(TimeBuffer *tb, int channel, int size, double duration) {
  tb->size  = size;
  tb->start = 0;
  tb->count = 0;
  tb->channel = channel;

  tb->total_counts = 0;

  tb->duration = duration;

  tb->times = (TimeType *)malloc(tb->size * sizeof(TimeType));
}

double tbGet(TimeBuffer *tb, int offset) {
  return(tb->times[(tb->start + offset) % tb->size].time);
}

double tbLastTime(TimeBuffer *tb) {
  return(tb->times[(tb->start + tb->count - 1) % tb->size].time);
}
 
void tbWrite(TimeBuffer *tb, double time) {
  ++ tb->total_counts;
  int end = (tb->start + tb->count) % tb->size;

  tb->times[end] = (TimeType){ time };

  ++ tb->count;
}

void tbPrune(TimeBuffer *tb, double time) {
  while (tb->count > 0) {
    if ((time - tb->times[tb->start].time) > tb->duration) {
      tb->start = (tb->start + 1) % tb->size;
      -- tb->count;
    }
    else break;
  }
}

#endif /* TIMEBUFFER_SEEN */
