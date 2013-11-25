#ifndef _PQ_FILEBUFFER_HEADER_SEEN
#define _PQ_FILEBUFFER_HEADER_SEEN

typedef struct {
  int64_t offset;
  
  uint64_t buffer_size;
  uint64_t buffer_fill;
  uint64_t num_read;
  int empty;

  ttd_t *buffered_records;
  int buffer_allocated;

  char *filename;
  int filename_allocated;

  int file_open;
  FILE *fp;
} pq_fb_t;

int pq_fb_init(pq_fb_t *buffer, uint64_t buffer_size, char* filename);
int pq_fb_openfile(pq_fb_t *buffer);
int pq_fb_cleanup(pq_fb_t *buffer);
ttd_t pq_fb_pop(pq_fb_t *buffer);


#endif //_PQ_FILEBUFFER_HEADER_SEEN
