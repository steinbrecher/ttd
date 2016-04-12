#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "ttd.h"
#include "ttd_ringbuffer.h"

#define RB_NEXT(rb, idx) (rb->start + rb->idx + 1) % rb->size

void ttd_rb_init(ttd_rb_t *rb, size_t size, uint64_t duration) {
  rb->times = NULL;

  rb->size  = size;
  rb->start = 0;
  rb->count = 0;

  rb->duration = duration;
  rb->times = (ttd_t *) calloc(rb->size, sizeof(ttd_t));
}

// Make sure to free ring buffers allocated with this function
ttd_rb_t *ttd_rb_build(size_t size, ttd_t duration) {
  ttd_rb_t *rb = (ttd_rb_t *) calloc(1, sizeof(ttd_rb_t));
  ttd_rb_init(rb, size, duration);
  return rb;
}

ttd_t ttd_rb_get(ttd_rb_t *rb, size_t offset) {
  return (rb->times[(rb->start + offset) % rb->size]);
}

/* ttd_t ttd_rb_peek(ttd_rb_t *rb) {
  return rb->times[rb->start];
}
 */

int_least16_t ttd_rb_del(ttd_rb_t *rb) {
  if (rb->count != 0) {
    rb->start = (rb->start + 1) % rb->size;
    --rb->count;
  }
  return 0;
}

int ttd_rb_insert(ttd_rb_t *rb, ttd_t time) {
  // Insert time into the buffer
  size_t end = (rb->start + rb->count) % rb->size;
  rb->times[end] = time;
  ++ rb->count;
  if (rb->count == (rb->size-1)) {
    ttd_rb_grow(rb);
    return(1);
  }
  return(0);
}

void ttd_rb_prune(ttd_rb_t *rb, ttd_t time) {
  while (rb->count > 0) {
    if ((time - rb->times[rb->start]) > rb->duration) {
      rb->start = (rb->start + 1) % rb->size;
      -- rb->count;
    }
    else break;
  }
}

void ttd_rb_cleanup(ttd_rb_t *rb) {
  if (rb == NULL) {return;}
  free(rb->times);
  rb->times = NULL;
}

int ttd_rb_grow(ttd_rb_t *rb) {
  ttd_t *newbuff;
  size_t i;
  if (rb->times != NULL) {
    printf("    Growing ringbuffer size to %lu\n", 2*rb->size);
    newbuff = (ttd_t *) malloc(2*rb->size*sizeof(ttd_t));
    if (newbuff == NULL) {
      printf("ERROR: Could not allocate larger ringbuffer\n");
      exit(-1);
    }
    // Copy memory in order, accounting for wrapping, to beginning of new buffer
    for (i=0; i<rb->count; i++) {
      newbuff[i] = ttd_rb_get(rb, i);
    }
    // Move head to point at start of newbuff
    rb->start = 0;
    // Update the size field in the ringbuffer
    rb->size = rb->size*2;
    // Free old buffer
    free(rb->times);
    // Point to new buffer
    rb->times = newbuff;
    return(0);
  }
  return(-1);
}



