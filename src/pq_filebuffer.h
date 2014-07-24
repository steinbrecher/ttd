#ifndef _PQ_FILEBUFFER_HEADER_SEEN
#define _PQ_FILEBUFFER_HEADER_SEEN

typedef struct {
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

  ////  Special pieces for the PicoQuant file buffer ////

  // PicoQuant-specific file information
  pq_fileinfo_t fileinfo;

  // Record conversion function (specialized per filetype)
  int (*to_ttd)(pq_rec_t, ttd_t *, uint64_t *, pq_fileinfo_t *);
} pq_fb_t;

int pq_fb_init(pq_fb_t *buffer, uint64_t buffer_size, char* filename);
int pq_fb_openfile(pq_fb_t *buffer);
int pq_fb_cleanup(pq_fb_t *buffer);
ttd_t pq_fb_pop(pq_fb_t *buffer);

#define PQ_FB_MALLOC_ERROR -1;
#define PQ_FB_FILE_OPEN_ERROR -2;

#endif //_PQ_FILEBUFFER_HEADER_SEEN
