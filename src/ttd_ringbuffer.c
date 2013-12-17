#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ttp_cli.h"
#include "ttd.h"
#include "ttd_ringbuffer.h"

void ttd_rb_init(ttd_rb_t *rb, int size, uint64_t duration) {
  rb->size  = size;
  rb->start = 0;
  rb->count = 0;

  rb->duration = duration;
  rb->times = (ttd_t *) malloc(rb->size * sizeof(ttd_t));
  rb->times_allocated = 1;
  if (ttp_cli_args.verbose) {
    printf("ttd_rb_init: Initialized ringbuffer with size %d and duration %" PRIu64 "\n", size, duration);
  }
}

// Make sure to free ring buffers allocated with this function
ttd_rb_t *ttd_rb_build(int size, ttd_t duration) {
  ttd_rb_t *rb = (ttd_rb_t *) malloc(sizeof(ttd_rb_t));
  ttd_rb_init(rb, size, duration);

  if (ttp_cli_args.verbose) {
    printf("ttd_rb_build: Built ringbuffer with size %d and duration %" PRIu64 "\n", size, duration);
  }
  return rb;
}

ttd_t ttd_rb_get(ttd_rb_t *rb, int offset) {
  return (rb->times[(rb->start + offset) % rb->size]);
}

void ttd_rb_insert(ttd_rb_t *rb, ttd_t time) {
  // Insert time into the buffer
  int end = (rb->start + rb->count) % rb->size;
  rb->times[end] = time;
  ++ rb->count;
  if (rb->count == (rb->size-1)) {
    ttd_rb_grow(rb);
  }
}

void ttd_rb_prune(ttd_rb_t *rb, ttd_t time) {
  while (rb->count > 0) {
    if ((time - ttd_rb_get(rb, 0)) > rb->duration) {
      rb->start = (rb->start + 1) % rb->size;
      -- rb->count;
    }
    else break;
  }
}

void ttd_rb_cleanup(ttd_rb_t *rb) {
  if (rb->times_allocated) {
    free(rb->times);
    rb->times_allocated = 0;
  }
}

int ttd_rb_grow(ttd_rb_t *rb) {
  ttd_t *newbuff;
  if (rb->times_allocated == 1) {
    newbuff = (ttd_t *) malloc(2*rb->size*sizeof(ttd_t));
    memcpy(newbuff, rb->times, rb->size);
    rb->size = rb->size*2;
    free(rb->times);
    rb->times = newbuff;
    return(0);
  }
  return(-1);
}



