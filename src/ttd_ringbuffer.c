#define __STDC_FORMAT_MACROS
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
}

ttd_rb_t *ttd_rb_build(int size, ttd_t duration) {
  // Make sure to free ring buffers allocated with this function
  ttd_rb_t *rb = (ttd_rb_t *) malloc(sizeof(ttd_rb_t));
  ttd_rb_init(rb, size, duration);
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
  



