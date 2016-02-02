#ifndef _TTD_RINGBUFFER_HEADER_SEEN
#define _TTD_RINGBUFFER_HEADER_SEEN

typedef struct {
  ttd_t *times;
  ttd_t duration;

  int size;		      // Max number of elems 
  int start;		      // Index of oldest element 
  int count;	      // Number of active elements 

  int times_allocated;
  int allocated;

} ttd_rb_t;

void ttd_rb_init(ttd_rb_t *rb, int size, ttd_t duration);

ttd_rb_t *ttd_rb_build(int size, ttd_t duration);

ttd_t ttd_rb_get(ttd_rb_t *rb, int offset);

ttd_t ttd_rb_peek(ttd_rb_t *rb);

void ttd_rb_insert(ttd_rb_t *rb, ttd_t time);

void ttd_rb_prune(ttd_rb_t *rb, ttd_t time);

void ttd_rb_cleanup(ttd_rb_t *rb);

int ttd_rb_grow(ttd_rb_t *rb);

int_least16_t ttd_rb_del(ttd_rb_t *rb);

#endif // _TTD_RINGBUFFER_HEADER_SEEN
