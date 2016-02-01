#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ttd.h"
#include "ttd_ringbuffer.h"

void ttd_rb_init(ttd_rb_t *rb, int size, uint64_t duration) {
  rb->size  = size;
  rb->start = 0;
  rb->count = 0;

  rb->duration = duration;
  rb->times = (ttd_t *) malloc(rb->size * sizeof(ttd_t));
  rb->times_allocated = 1;
}

// Make sure to free ring buffers allocated with this function
ttd_rb_t *ttd_rb_build(int size, ttd_t duration) {
  ttd_rb_t *rb = (ttd_rb_t *) malloc(sizeof(ttd_rb_t));
  ttd_rb_init(rb, size, duration);

  return rb;
}

ttd_t ttd_rb_get(ttd_rb_t *rb, int offset) {
  return (rb->times[(rb->start + offset) % rb->size]);
}

int_least16_t ttd_rb_del(ttd_rb_t *rb) {
  if (rb->count != 0) {
    rb->start = (rb->start + 1) % rb->size;
    --rb->count;
  }
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
//    if (time < ttd_rb_get(rb,0)) {
//      break;
//    }

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
    printf("Growing ringbuffer size to %d\n", rb->size);
    newbuff = (ttd_t *) malloc(2*rb->size*sizeof(ttd_t));
    if (newbuff == NULL) {
      printf("ERROR: Could not allocate larger ringbuffer\n");
      exit(-1);
    }
    memcpy(newbuff, rb->times, rb->size);
    rb->size = rb->size*2;
    free(rb->times);
    rb->times = newbuff;
    return(0);
  }
  return(-1);
}



