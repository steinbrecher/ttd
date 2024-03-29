#ifndef _TTD_RINGBUFFER_HEADER_SEEN
#define _TTD_RINGBUFFER_HEADER_SEEN

#include "ttd.h"

//static ttd_t rb_duration;

typedef struct {
    size_t size;          // Max number of elems
    size_t start;          // Index of oldest element
    size_t count;        // Number of active elements
    ttd_t duration;

    //int allocated;
    //ttd_t duration;
    ttd_t *times;
} ttd_rb_t;

void ttd_rb_init(ttd_rb_t *rb, size_t size, ttd_t duration);

ttd_rb_t *ttd_rb_build(size_t size, ttd_t duration);

ttd_t ttd_rb_get(ttd_rb_t *rb, size_t offset);

//ttd_t ttd_rb_peek(ttd_rb_t *rb);

int ttd_rb_insert(ttd_rb_t *rb, ttd_t time);

void ttd_rb_prune(ttd_rb_t *rb, ttd_t time);

void ttd_rb_cleanup(ttd_rb_t *rb);

int ttd_rb_grow(ttd_rb_t *rb);

int_least16_t ttd_rb_del(ttd_rb_t *rb);

#endif // _TTD_RINGBUFFER_HEADER_SEEN
