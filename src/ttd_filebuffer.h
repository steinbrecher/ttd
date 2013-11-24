#ifndef _TTD_FILEBUFFER_HEADER_SEEN
#define _TTD_FILEBUFFER_HEADER_SEEN
typedef struct {
  int64_t offset;
  
  uint64_t buffer_size;
  uint64_t buffer_fill;
  uint64_t num_read;
  int empty;

  ttd_t *buffered_records;
  int buffer_allocated;

  int file_open;
  char *filename;
  FILE *fp;
} ttd_fb_t;

int ttd_fb_init(ttd_fb_t *buffer, uint64_t buffer_size, char* filename);
int ttd_fb_openfile(ttd_fb_t *buffer);
int ttd_fb_cleanup(ttd_fb_t *buffer);
ttd_t ttd_fb_pop(ttd_fb_t *buffer);

#endif // _TTD_FILEBUFFER_HEADER_SEEN

